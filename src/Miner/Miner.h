// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2014-2017 XDN-project developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <atomic>
#include <list>
#include <mutex>
#include <thread>

#include <System/Dispatcher.h>
#include <System/Event.h>
#include <System/RemoteContext.h>

#include "CryptoNote.h"
#include "CryptoNoteCore/Difficulty.h"

#include "Logging/LoggerRef.h"
#include "CryptoNoteCore/OnceInInterval.h"

namespace CryptoNote {

struct BlockMiningParameters {
  Block blockTemplate;
  difficulty_type difficulty;
};

class Miner {
public:
  Miner(System::Dispatcher& dispatcher, Logging::ILogger& logger);
  ~Miner();

  Block mine(const BlockMiningParameters& blockMiningParameters, size_t threadCount);

  //NOTE! this is blocking method
  void stop();

private:
  System::Dispatcher& m_dispatcher;
  System::Event m_miningStopped;
//$$$$
    OnceInInterval m_update_merge_hr_interval;
    bool m_do_print_hashrate;
    std::atomic<uint64_t> m_last_hr_merge_time;
    std::atomic<uint64_t> m_prev_block_time;
    std::atomic<uint64_t> m_hashes_total;
    std::atomic<uint64_t> m_hashes;
    std::atomic<uint64_t> m_current_hash_rate;
    std::mutex m_last_hash_rates_lock;
    std::list<uint64_t> m_last_hash_rates;	
//$$$$
  enum class MiningState : uint8_t { MINING_STOPPED, BLOCK_FOUND, MINING_IN_PROGRESS};
  std::atomic<MiningState> m_state;

  std::vector<std::unique_ptr<System::RemoteContext<void>>>  m_workers;

  Block m_block;

  Logging::LoggerRef m_logger;

  void runWorkers(BlockMiningParameters blockMiningParameters, size_t threadCount);
  void workerFunc(const Block& blockTemplate, difficulty_type difficulty, uint32_t nonceStep);
  bool setStateBlockFound();
//$$$$
    void  merge_hr();
//$$$$
  
};

} //namespace CryptoNote
