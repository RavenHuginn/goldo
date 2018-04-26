// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2014-2017 XDN-project developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Miner.h"

#include <functional>
#include <future>
#include <numeric>
#include <sstream>
#include <thread>

#include "crypto/crypto.h"
#include "CryptoNoteCore/CryptoNoteFormatUtils.h"
#include "CryptoNoteConfig.h"

#include <System/InterruptedException.h>

#include "CryptoNote.h"

#include "Common/ConsoleTools.h"
#include "zrainbow.h"
#include "ztime.h"

namespace CryptoNote {

///////////////////////////////////////////////////////////////////////
uint64_t millisecondsSinceEpoch() {
	auto now = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Miner::Miner(System::Dispatcher& dispatcher, Logging::ILogger& logger) :
  m_dispatcher(dispatcher),
  m_miningStopped(dispatcher),
  m_state(MiningState::MINING_STOPPED),
  m_logger(logger, "Miner"),
    m_last_hr_merge_time(0),
    m_hashes(0),
	m_hashes_total(0),
	m_prev_block_time(0),
    m_do_print_hashrate(false),
    m_current_hash_rate(0), 
    m_update_merge_hr_interval(2)
  {
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Miner::~Miner() {
  assert(m_state != MiningState::MINING_IN_PROGRESS);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Block Miner::mine(const BlockMiningParameters& blockMiningParameters, size_t threadCount) {
  if (threadCount == 0) {
    throw std::runtime_error("Miner requires at least one thread");
  }

  if (m_state == MiningState::MINING_IN_PROGRESS) {
    throw std::runtime_error("Mining is already in progress");
  }

  m_state = MiningState::MINING_IN_PROGRESS;
  m_miningStopped.clear();

  runWorkers(blockMiningParameters, threadCount);

  assert(m_state != MiningState::MINING_IN_PROGRESS);
  if (m_state == MiningState::MINING_STOPPED) {
    m_logger(Logging::DEBUGGING) << "Mining has been stopped";
    throw System::InterruptedException();
  }

  assert(m_state == MiningState::BLOCK_FOUND);
  return m_block;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Miner::stop() {
  MiningState state = MiningState::MINING_IN_PROGRESS;

  if (m_state.compare_exchange_weak(state, MiningState::MINING_STOPPED)) {
    m_miningStopped.wait();
    m_miningStopped.clear();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////
void Miner::runWorkers(BlockMiningParameters blockMiningParameters, size_t threadCount) {
	
	assert(threadCount > 0);

	uint32_t height = boost::get<BaseInput>(blockMiningParameters.blockTemplate.baseTransaction.inputs.front()).blockIndex;

	uint64_t hr = blockMiningParameters.difficulty / CryptoNote::parameters::DIFFICULTY_TARGET;
	float khr = static_cast<float>(hr)/static_cast<float>(1000);
	
	std::cout 
		<< khaki << "Network: "
		<< std::setprecision(2) << std::fixed << std::setw(9) << yellow << khr << khaki << " KH/s hashrate," 
		<< " block is " << yellow << height
		<< khaki << " @ " << z_get_now_time_str("%Y-%m-%d %H:%M:%S")
		<< khaki << ", difficulty is " << blockMiningParameters.difficulty 
		<< grey << std::endl;
		
	m_update_merge_hr_interval.call([&](){ merge_hr(); return true;});
	m_prev_block_time = millisecondsSinceEpoch();
    
  try {
    blockMiningParameters.blockTemplate.nonce = Crypto::rand<uint32_t>();

    for (size_t i = 0; i < threadCount; ++i) {
      m_workers.emplace_back(std::unique_ptr<System::RemoteContext<void>> (
        new System::RemoteContext<void>(m_dispatcher, std::bind(&Miner::workerFunc, this, blockMiningParameters.blockTemplate, blockMiningParameters.difficulty, threadCount)))
      );
		std::cout << navy << "-";
      blockMiningParameters.blockTemplate.nonce++;
    }
	std::cout << grey << std::endl;
    m_workers.clear();

  } catch (std::exception& e) {
    m_logger(Logging::ERROR, Logging::BRIGHT_RED) << "Error occured during mining: " << e.what();
    m_state = MiningState::MINING_STOPPED;
  }

  m_miningStopped.set();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Miner::workerFunc(const Block& blockTemplate, difficulty_type difficulty, uint32_t nonceStep) {
	try {
		Block block = blockTemplate;
		Crypto::cn_context cryptoContext;

		while (m_state == MiningState::MINING_IN_PROGRESS) {
			
			Crypto::Hash hash;
			
			if (!get_block_longhash(cryptoContext, block, hash)) { //error occured		
			
				m_logger(Logging::DEBUGGING, Logging::RED) << "calculating long hash error occured";
				m_state = MiningState::MINING_STOPPED;
				return;
			}

			if (check_hash(hash, difficulty)) {
				
				std::cout << yellow << "Block found for difficulty: " << red << difficulty << grey << std::endl;

				if (!setStateBlockFound()) {
					
					m_logger(Logging::DEBUGGING, Logging::RED) << "Block is already found or mining stopped";
					return;
				}

				m_block = block;
				return;
			}

			block.nonce += nonceStep;
			//$$$$
			++m_hashes;
			//$$$$
		}
	}
	catch (std::exception& e) {
		m_logger(Logging::ERROR, Logging::BRIGHT_RED) << "Miner got error: " << e.what();
		m_state = MiningState::MINING_STOPPED;
	}
}
//-----------------------------------------------------------------------------------------------------
void Miner::merge_hr() {

	m_hashes_total += m_hashes;
	
	if(m_last_hr_merge_time) {
		
		m_current_hash_rate = m_hashes * 1000 / (millisecondsSinceEpoch() - m_last_hr_merge_time + 1);
		std::lock_guard<std::mutex> lk(m_last_hash_rates_lock);
		m_last_hash_rates.push_back(m_current_hash_rate);
		if(m_last_hash_rates.size() > 30)
		m_last_hash_rates.pop_front();

		uint64_t total_hr = std::accumulate(m_last_hash_rates.begin(), m_last_hash_rates.end(), static_cast<uint64_t>(0));
			
		float hr = (static_cast<float>(total_hr)/static_cast<float>(m_last_hash_rates.size())/1000);
		
		uint32_t prec;
		if (hr < 0.09) {
			prec = 3;
		}
		else {
			prec = 2;
		}
		
		std::cout 
			<< green << "Miner:   " << std::setprecision(prec) << std::fixed << std::setw(9) << lime << hr << green 
			<< " KH/s hashrate, " << lime << m_hashes_total << green 
			<< " hashes mined total, the time spent is " 
			<< std::setprecision(0) << lime << ((millisecondsSinceEpoch() - m_prev_block_time)/1000)
			<< green << " sec." << grey << std::endl;
	}
	else { 
		std::cout << green << "Miner:      " << maroon << "No stats yet" << grey << std::endl;
	}

	m_last_hr_merge_time = millisecondsSinceEpoch();
	m_hashes = 0;
}
//-----------------------------------------------------------------------------------------------------
 bool Miner::setStateBlockFound() {
  auto state = m_state.load();

  for (;;) {
    switch (state) {
      case MiningState::BLOCK_FOUND:
        return false;

      case MiningState::MINING_IN_PROGRESS:
        if (m_state.compare_exchange_weak(state, MiningState::BLOCK_FOUND)) {
          return true;
        }
        break;

      case MiningState::MINING_STOPPED:
        return false;

      default:
        assert(false);
        return false;
    }
  }
}

} //namespace CryptoNote
