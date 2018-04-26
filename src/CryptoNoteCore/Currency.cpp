// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2014-2017 XDN-project developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Currency.h"
#include <cctype>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include "../Common/Base58.h"
#include "../Common/int-util.h"
#include "../Common/StringTools.h"

#include "Account.h"
#include "CryptoNoteBasicImpl.h"
#include "CryptoNoteFormatUtils.h"
#include "CryptoNoteTools.h"
#include "TransactionExtra.h"
#include "UpgradeDetector.h"

#include "Common/ConsoleTools.h"
#include "zrainbow.h"

#undef ERROR

using namespace Logging;
using namespace Common;

namespace CryptoNote {

const std::vector<uint64_t> Currency::PRETTY_AMOUNTS = {
  1, 2, 3, 4, 5, 6, 7, 8, 9,
  10, 20, 30, 40, 50, 60, 70, 80, 90,
  100, 200, 300, 400, 500, 600, 700, 800, 900,
  1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000,
  10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000,
  100000, 200000, 300000, 400000, 500000, 600000, 700000, 800000, 900000,
  1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 7000000, 8000000, 9000000,
  10000000, 20000000, 30000000, 40000000, 50000000, 60000000, 70000000, 80000000, 90000000,
  100000000, 200000000, 300000000, 400000000, 500000000, 600000000, 700000000, 800000000, 900000000,
  1000000000, 2000000000, 3000000000, 4000000000, 5000000000, 6000000000, 7000000000, 8000000000, 9000000000,
  10000000000, 20000000000, 30000000000, 40000000000, 50000000000, 60000000000, 70000000000, 80000000000, 90000000000,
  100000000000, 200000000000, 300000000000, 400000000000, 500000000000, 600000000000, 700000000000, 800000000000, 900000000000,
  1000000000000, 2000000000000, 3000000000000, 4000000000000, 5000000000000, 6000000000000, 7000000000000, 8000000000000, 9000000000000,
  10000000000000, 20000000000000, 30000000000000, 40000000000000, 50000000000000, 60000000000000, 70000000000000, 80000000000000, 90000000000000,
  100000000000000, 200000000000000, 300000000000000, 400000000000000, 500000000000000, 600000000000000, 700000000000000, 800000000000000, 900000000000000,
  1000000000000000, 2000000000000000, 3000000000000000, 4000000000000000, 5000000000000000, 6000000000000000, 7000000000000000, 8000000000000000, 9000000000000000,
  10000000000000000, 20000000000000000, 30000000000000000, 40000000000000000, 50000000000000000, 60000000000000000, 70000000000000000, 80000000000000000, 90000000000000000,
  100000000000000000, 200000000000000000, 300000000000000000, 400000000000000000, 500000000000000000, 600000000000000000, 700000000000000000, 800000000000000000, 900000000000000000,
  1000000000000000000, 2000000000000000000, 3000000000000000000, 4000000000000000000, 5000000000000000000, 6000000000000000000, 7000000000000000000, 8000000000000000000, 9000000000000000000,
  10000000000000000000ull
};

bool Currency::init() {
  if (!generateGenesisBlock()) {
    logger(ERROR, BRIGHT_RED) << "Failed to generate genesis block";
    return false;
  }

  if (!get_block_hash(m_genesisBlock, m_genesisBlockHash)) {
    logger(ERROR, BRIGHT_RED) << "Failed to get genesis block hash";
    return false;
  }

  if (isTestnet()) {
    m_upgradeHeightV2 = 0;
    m_upgradeHeightV3 = static_cast<uint32_t>(-1);
    m_upgradeHeightV4 = static_cast<uint32_t>(-1);
    m_blocksFileName = "testnet_" + m_blocksFileName;
    m_blocksCacheFileName = "testnet_" + m_blocksCacheFileName;
    m_blockIndexesFileName = "testnet_" + m_blockIndexesFileName;
    m_txPoolFileName = "testnet_" + m_txPoolFileName;
    m_blockchinIndicesFileName = "testnet_" + m_blockchinIndicesFileName;
  }

  return true;
}

bool Currency::generateGenesisBlock() {
  m_genesisBlock = boost::value_initialized<Block>();

  // Hard code coinbase tx in genesis block, because "tru" generating tx use random, but genesis should be always the same
  std::string genesisCoinbaseTxHex = GENESIS_COINBASE_TX_HEX;
  BinaryArray minerTxBlob;

  bool r =
    fromHex(genesisCoinbaseTxHex, minerTxBlob) &&
    fromBinaryArray(m_genesisBlock.baseTransaction, minerTxBlob);

  if (!r) {
    logger(ERROR, BRIGHT_RED) << "failed to parse coinbase tx from hard coded blob";
    return false;
  }

  m_genesisBlock.majorVersion = BLOCK_MAJOR_VERSION_1;
  m_genesisBlock.minorVersion = BLOCK_MINOR_VERSION_0;
  m_genesisBlock.timestamp = 0;
  m_genesisBlock.nonce = GENESIS_NONCE;
  if (m_testnet) {
    ++m_genesisBlock.nonce;
  }
  //miner::find_nonce_for_given_block(bl, 1, 0);

  return true;
}

////////////////////////////////////////////////////////////////////////////
uint64_t Currency::baseRewardFunction(uint64_t alreadyGeneratedCoins, uint32_t height, const Crypto::Hash last_BlockHash) const 
{
//logger(TRACE) << "FN = baseRewardFunction, height = " << height << " Prev Hash = " << last_BlockHash;
	uint64_t base_reward;
	uint64_t prize_factor = 0;
	
	prize_factor = Lottery(last_BlockHash);
	
	if (prize_factor>1)
			base_reward = parameters::RUSH_REWARD_Z*prize_factor*5;
	else
			base_reward = parameters::RUSH_REWARD_Z*prize_factor;
	
	if (height==0) 
	{
		base_reward = 0;
	}
	else if (height==1) 
	{
		base_reward = parameters::GENESIS_BLOCK_REWARD_Z;
	}
	else if (height>parameters::RUSH_BLOCKS_Z+1) 
	{
		base_reward = parameters::COMMON_REWARD_Z*prize_factor;
	}
//////////
	if (alreadyGeneratedCoins>parameters::MONEY_SUPPLY_LOWSPEED_Z) 
	{
		base_reward = base_reward / 100;
	}
//////////	
	if (alreadyGeneratedCoins>parameters::MONEY_SUPPLY_MINEABLE_Z) 
	{
		base_reward = 1;
	}
	
	return base_reward;
}////////////////////////////////////////////////////////////////////////////
uint64_t Currency::Lottery(const Crypto::Hash lbh) const 
{
	std::string sh;
	uint8_t i;
	uint8_t bingo[16];
	uint64_t factor = 1;
	uint8_t level = 0;
	
	sh = toHex(lbh.data,32);

	for (i = 0; i <16 ; i++) 
		{
			bingo[i] = 0;
		}
	
	bingo[fromHex(sh[6])]+=1;
	bingo[fromHex(sh[12])]+=1;
	bingo[fromHex(sh[18])]+=1;
	bingo[fromHex(sh[24])]+=1;
	bingo[fromHex(sh[30])]+=1;
	bingo[fromHex(sh[36])]+=1;
	bingo[fromHex(sh[42])]+=1;
	
	for (i = 0; i <16 ; i++) 
		{
			if (level < bingo[i])
				{
					level = bingo[i];
				}				
		}

	switch (level) 
	{
		case 1: 
			{
				factor = 1;
				break;
			}
		case 2:
			{
				factor = 1;
				break;
			}
		case 3: 
			{
				factor = 2;
				break;
			}
		case 4:
			{
				factor = 100;
				break;
			}
		case 5:
			{
				factor = 1000;
				break;
			}
		case 6:
			{
				factor = 10000;
				break;
			}
		case 7:
			{
				factor = 100000;
				break;
			}
	}		
	
	logger(TRACE) << "LEVEL = " << to_string(level) << " FACTOR = " << factor;
	return factor;
}
////////////////////////////////////////////////////////////////////////////

size_t Currency::blockGrantedFullRewardZoneByBlockVersion(uint8_t blockMajorVersion) const {
  if (blockMajorVersion >= BLOCK_MAJOR_VERSION_4) {
    return m_blockGrantedFullRewardZone;
  } else {
    return CryptoNote::parameters::CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V1;
  }
}

uint32_t Currency::upgradeHeight(uint8_t majorVersion) const {
  if (majorVersion == BLOCK_MAJOR_VERSION_2) {
    return m_upgradeHeightV2;
  } else if (majorVersion == BLOCK_MAJOR_VERSION_3) {
    return m_upgradeHeightV3;
  } else if (majorVersion == BLOCK_MAJOR_VERSION_4) {
    return m_upgradeHeightV4;
  } else {
    return static_cast<uint32_t>(-1);
  }
}

bool Currency::getBlockReward(size_t medianSize, size_t currentBlockSize, uint64_t alreadyGeneratedCoins,
  uint64_t fee, uint32_t height, uint64_t& reward, int64_t& emissionChange, const Crypto::Hash last_BlockHash) const {
  assert(alreadyGeneratedCoins <= m_moneySupply);

  uint64_t baseReward = baseRewardFunction(alreadyGeneratedCoins, height, last_BlockHash);

  size_t blockGrantedFullRewardZone = (height < parameters::UPGRADE_HEIGHT_V4) ? parameters::CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V1 : m_blockGrantedFullRewardZone;
  medianSize = std::max(medianSize, blockGrantedFullRewardZone);
  if (currentBlockSize > UINT64_C(2) * medianSize) {
    logger(TRACE) << "Block cumulative size is too big: " << currentBlockSize << ", expected less than " << 2 * medianSize;
    return false;
  }

  uint64_t penalizedBaseReward = getPenalizedAmount(baseReward, medianSize, currentBlockSize);

  emissionChange = penalizedBaseReward;
  reward = penalizedBaseReward + fee;

  return true;
}

uint64_t Currency::calculateInterest(uint64_t amount, uint32_t term) const {
  assert(m_depositMinTerm <= term && term <= m_depositMaxTerm);
  assert(static_cast<uint64_t>(term)* m_depositMaxTotalRate > m_depositMinTotalRateFactor);

  uint64_t a = static_cast<uint64_t>(term) * m_depositMaxTotalRate - m_depositMinTotalRateFactor;
  uint64_t bHi;
  uint64_t bLo = mul128(amount, a, &bHi);

  uint64_t interestHi;
  uint64_t interestLo;
  assert(std::numeric_limits<uint32_t>::max() / 100 > m_depositMaxTerm);
  div128_32(bHi, bLo, static_cast<uint32_t>(100 * m_depositMaxTerm), &interestHi, &interestLo);
  assert(interestHi == 0);

  return interestLo;
}

uint64_t Currency::calculateTotalTransactionInterest(const Transaction& tx) const {
  uint64_t interest = 0;
  for (const TransactionInput& input : tx.inputs) {
    if (input.type() == typeid(MultisignatureInput)) {
      const MultisignatureInput& multisignatureInput = boost::get<MultisignatureInput>(input);
      if (multisignatureInput.term != 0) {
        interest += calculateInterest(multisignatureInput.amount, multisignatureInput.term);
      }
    }
  }

  return interest;
}

uint64_t Currency::getTransactionInputAmount(const TransactionInput& in) const {
  if (in.type() == typeid(KeyInput)) {
    return boost::get<KeyInput>(in).amount;
  } else if (in.type() == typeid(MultisignatureInput)) {
    const MultisignatureInput& multisignatureInput = boost::get<MultisignatureInput>(in);
    if (multisignatureInput.term == 0) {
      return multisignatureInput.amount;
    } else {
      return multisignatureInput.amount + calculateInterest(multisignatureInput.amount, multisignatureInput.term);
    }
  } else if (in.type() == typeid(BaseInput)) {
    return 0;
  } else {
    assert(false);
    return 0;
  }
}

uint64_t Currency::getTransactionAllInputsAmount(const Transaction& tx) const {
  uint64_t amount = 0;
  for (const auto& in : tx.inputs) {
    amount += getTransactionInputAmount(in);
  }
  return amount;
}

bool Currency::getTransactionFee(const Transaction& tx, uint64_t & fee) const {
  uint64_t amount_in = 0;
  uint64_t amount_out = 0;

  for (const auto& in : tx.inputs) {
    amount_in += getTransactionInputAmount(in);
  }

  for (const auto& o : tx.outputs) {
    amount_out += o.amount;
  }

  if (amount_in < amount_out) {
    return false;
  }

  fee = amount_in - amount_out;
  return true;
}

uint64_t Currency::getTransactionFee(const Transaction& tx) const {
  uint64_t r = 0;
  if (!getTransactionFee(tx, r)) {
    r = 0;
  }
  return r;
}

size_t Currency::maxBlockCumulativeSize(uint64_t height) const {
  assert(height <= std::numeric_limits<uint64_t>::max() / m_maxBlockSizeGrowthSpeedNumerator);
  size_t maxSize = static_cast<size_t>(m_maxBlockSizeInitial +
    (height * m_maxBlockSizeGrowthSpeedNumerator) / m_maxBlockSizeGrowthSpeedDenominator);
  assert(maxSize >= m_maxBlockSizeInitial);
  return maxSize;
}

bool Currency::constructMinerTx(uint32_t height, size_t medianSize, uint64_t alreadyGeneratedCoins, size_t currentBlockSize,
  uint64_t fee, const AccountPublicAddress& minerAddress, Transaction& tx,
  const BinaryArray& extraNonce/* = BinaryArray()*/, size_t maxOuts/* = 1*/, const Crypto::Hash last_BlockHash) const {
  tx.inputs.clear();
  tx.outputs.clear();
  tx.extra.clear();
  
  size_t z_lock = 0;

/////////////////////////////
//logger(ERROR, BRIGHT_CYAN)<<"constructMinerTx "<< minerAddress.viewPublicKey <<" "<<minerAddress.spendPublicKey;
/////////////////////////////    
  KeyPair txkey = generateKeyPair();
  addTransactionPublicKeyToExtra(tx.extra, txkey.publicKey);
  if (!extraNonce.empty()) {
    if (!addExtraNonceToTransactionExtra(tx.extra, extraNonce)) {
      return false;
    }
  }

  BaseInput in;
  in.blockIndex = height;

  uint64_t blockReward;
  int64_t emissionChange;
  if (!getBlockReward(medianSize, currentBlockSize, alreadyGeneratedCoins, fee, height, blockReward, emissionChange, last_BlockHash)) {
    logger(INFO) << "Block is too big";
    return false;
  }

  std::vector<uint64_t> outAmounts;
  decompose_amount_into_digits(blockReward, m_defaultDustThreshold,
    [&outAmounts](uint64_t a_chunk) { outAmounts.push_back(a_chunk); },
    [&outAmounts](uint64_t a_dust) { outAmounts.push_back(a_dust); });

  if (!(1 <= maxOuts)) { logger(ERROR, BRIGHT_RED) << "max_out must be non-zero"; return false; }
  while (maxOuts < outAmounts.size()) {
    outAmounts[outAmounts.size() - 2] += outAmounts.back();
    outAmounts.resize(outAmounts.size() - 1);
  }

  uint64_t summaryAmounts = 0;
  for (size_t no = 0; no < outAmounts.size(); no++) {
    Crypto::KeyDerivation derivation = boost::value_initialized<Crypto::KeyDerivation>();
    Crypto::PublicKey outEphemeralPubKey = boost::value_initialized<Crypto::PublicKey>();

    bool r = Crypto::generate_key_derivation(minerAddress.viewPublicKey, txkey.secretKey, derivation);

    if (!(r)) {
      logger(ERROR, BRIGHT_RED)
        << "while creating outs: failed to generate_key_derivation("
        << minerAddress.viewPublicKey << ", " << txkey.secretKey << ")";
      return false;
    }

    r = Crypto::derive_public_key(derivation, no, minerAddress.spendPublicKey, outEphemeralPubKey);

    if (!(r)) {
      logger(ERROR, BRIGHT_RED)
        << "while creating outs: failed to derive_public_key("
        << derivation << ", " << no << ", "
        << minerAddress.spendPublicKey << ")";
      return false;
    }

    KeyOutput tk;
    tk.key = outEphemeralPubKey;

    TransactionOutput out;
    summaryAmounts += out.amount = outAmounts[no];
    out.target = tk;
    tx.outputs.push_back(out);
  }

  if (!(summaryAmounts == blockReward)) {
    logger(ERROR, BRIGHT_RED) << "Failed to construct miner tx, summaryAmounts = " << summaryAmounts << " not equal blockReward = " << blockReward;
    return false;
  }

  tx.version = TRANSACTION_VERSION_1;
  //lock
//RUSH_BLOCKS_Z must be locked longer 
	
	if(height<2)
		z_lock = height + 1;
	else if (height < parameters::RUSH_BLOCKS_Z + 2)
		z_lock = height + parameters::RUSH_REWARD_LOCK_Z;
	else
		z_lock = height + m_minedMoneyUnlockWindow;
	
	tx.unlockTime = z_lock;
//////////////////////
  tx.inputs.push_back(in);
  return true;
}

bool Currency::isFusionTransaction(const std::vector<uint64_t>& inputsAmounts, const std::vector<uint64_t>& outputsAmounts, size_t size) const {
  if (size > fusionTxMaxSize()) {
    return false;
  }

  if (inputsAmounts.size() < fusionTxMinInputCount()) {
    return false;
  }

  if (inputsAmounts.size() < outputsAmounts.size() * fusionTxMinInOutCountRatio()) {
    return false;
  }

  uint64_t inputAmount = 0;
  for (auto amount: inputsAmounts) {
    if (amount < defaultDustThreshold()) {
      return false;
    }

    inputAmount += amount;
  }

  std::vector<uint64_t> expectedOutputsAmounts;
  expectedOutputsAmounts.reserve(outputsAmounts.size());
  decomposeAmount(inputAmount, defaultDustThreshold(), expectedOutputsAmounts);
  std::sort(expectedOutputsAmounts.begin(), expectedOutputsAmounts.end());

  return expectedOutputsAmounts == outputsAmounts;
}

bool Currency::isFusionTransaction(const Transaction& transaction, size_t size) const {
  assert(getObjectBinarySize(transaction) == size);

  std::vector<uint64_t> outputsAmounts;
  outputsAmounts.reserve(transaction.outputs.size());
  for (const TransactionOutput& output : transaction.outputs) {
    outputsAmounts.push_back(output.amount);
  }

  return isFusionTransaction(getInputsAmounts(transaction), outputsAmounts, size);
}

bool Currency::isFusionTransaction(const Transaction& transaction) const {
  return isFusionTransaction(transaction, getObjectBinarySize(transaction));
}

bool Currency::isAmountApplicableInFusionTransactionInput(uint64_t amount, uint64_t threshold) const {
  uint8_t ignore;
  return isAmountApplicableInFusionTransactionInput(amount, threshold, ignore);
}

bool Currency::isAmountApplicableInFusionTransactionInput(uint64_t amount, uint64_t threshold, uint8_t& amountPowerOfTen) const {
  if (amount >= threshold) {
    return false;
  }

  if (amount < defaultDustThreshold()) {
    return false;
  }

  auto it = std::lower_bound(PRETTY_AMOUNTS.begin(), PRETTY_AMOUNTS.end(), amount);
  if (it == PRETTY_AMOUNTS.end() || amount != *it) {
    return false;
  } 

  amountPowerOfTen = static_cast<uint8_t>(std::distance(PRETTY_AMOUNTS.begin(), it) / 9);
  return true;
}

std::string Currency::accountAddressAsString(const AccountBase& account) const {
  return getAccountAddressAsStr(m_publicAddressBase58Prefix, account.getAccountKeys().address);
}

std::string Currency::accountAddressAsString(const AccountPublicAddress& accountPublicAddress) const {
  return getAccountAddressAsStr(m_publicAddressBase58Prefix, accountPublicAddress);
}

bool Currency::parseAccountAddressString(const std::string& str, AccountPublicAddress& addr) const {
  uint64_t prefix;
  if (!CryptoNote::parseAccountAddressString(prefix, addr, str)) {
    return false;
  }

  if (prefix != m_publicAddressBase58Prefix) {
    logger(DEBUGGING) << "Wrong address prefix: " << prefix << ", expected " << m_publicAddressBase58Prefix;
    return false;
  }

  return true;
}

std::string Currency::formatAmount(uint64_t amount) const {
  std::string s = std::to_string(amount);
  if (s.size() < m_numberOfDecimalPlaces + 1) {
    s.insert(0, m_numberOfDecimalPlaces + 1 - s.size(), '0');
  }
  s.insert(s.size() - m_numberOfDecimalPlaces, ".");
  return s;
}

std::string Currency::formatAmount(int64_t amount) const {
  std::string s = formatAmount(static_cast<uint64_t>(std::abs(amount)));

  if (amount < 0) {
    s.insert(0, "-");
  }

  return s;
}

bool Currency::parseAmount(const std::string& str, uint64_t& amount) const {
  std::string strAmount = str;
  boost::algorithm::trim(strAmount);

  size_t pointIndex = strAmount.find_first_of('.');
  size_t fractionSize;
  if (std::string::npos != pointIndex) {
    fractionSize = strAmount.size() - pointIndex - 1;
    while (m_numberOfDecimalPlaces < fractionSize && '0' == strAmount.back()) {
      strAmount.erase(strAmount.size() - 1, 1);
      --fractionSize;
    }
    if (m_numberOfDecimalPlaces < fractionSize) {
      return false;
    }
    strAmount.erase(pointIndex, 1);
  } else {
    fractionSize = 0;
  }

  if (strAmount.empty()) {
    return false;
  }

  if (!std::all_of(strAmount.begin(), strAmount.end(), ::isdigit)) {
    return false;
  }

  if (fractionSize < m_numberOfDecimalPlaces) {
    strAmount.append(m_numberOfDecimalPlaces - fractionSize, '0');
  }

  return Common::fromString(strAmount, amount);
}
//******************************************************************************************************************
difficulty_type Currency::nextDifficultyOld(
	std::vector<uint64_t> timestamps,
	std::vector<difficulty_type> cumulativeDifficulties, 
	size_t height) const {

//#1	
	assert(m_difficultyWindow >= 2);

	if (timestamps.size() > m_difficultyWindow) {
		timestamps.resize(m_difficultyWindow);
		cumulativeDifficulties.resize(m_difficultyWindow);
	}

	size_t length = timestamps.size();
	assert(length == cumulativeDifficulties.size());
	assert(length <= m_difficultyWindow);
	if (length <= 1) {
		return 1;
	}

	sort(timestamps.begin(), timestamps.end());

	size_t cutBegin, cutEnd;
//#2  
	assert(2 * m_difficultyCut <= m_difficultyWindow - 2);
	
	if (length <= m_difficultyWindow - 2 * m_difficultyCut) {
		cutBegin = 0;
		cutEnd = length;
	} 
	else {
		cutBegin = (length - (m_difficultyWindow - 2 * m_difficultyCut) + 1) / 2;
		cutEnd = cutBegin + (m_difficultyWindow - 2 * m_difficultyCut);
	}
//#3  
	assert(/*cut_begin >= 0 &&*/ cutBegin + 2 <= cutEnd && cutEnd <= length);
	
	uint64_t timeSpan = timestamps[cutEnd - 1] - timestamps[cutBegin];
	if (timeSpan == 0) {
		timeSpan = 1;
	}
//#4
	difficulty_type totalWork = cumulativeDifficulties[cutEnd - 1] - cumulativeDifficulties[cutBegin];
	assert(totalWork > 0);

	uint64_t low, high;
	low = mul128(totalWork, m_difficultyTarget, &high);
//#5  
	if (high != 0 || low + timeSpan - 1 < low) {
		return 0;
	}
//#6
	difficulty_type new_diff = (low + timeSpan - 1) / timeSpan;
//std::cout << green << "DIFF BASE: " << new_diff << std::endl;
	if (height>(parameters::UPGRADE_HEIGHT_V4+1)) 
		{
		new_diff += new_diff*(
			(parameters::EXPECTED_NUMBER_OF_BLOCKS_PER_DAY+height)/parameters::EXPECTED_NUMBER_OF_BLOCKS_PER_DAY);
		}
//std::cout << maroon << "DIFF UPGR: " << new_diff << std::endl;		
	return new_diff;
}
//******************************************************************************************************************
//******************************************************************************************************************
difficulty_type Currency::nextDifficultyZawy(
	std::vector<uint64_t> _timestamps,
	std::vector<difficulty_type> _cumulativeDifficulties,
	size_t _difficultyWindow) const {

	size_t z_difficultyCut = 0;
	
//#1
    assert(_difficultyWindow >= 2);

    size_t t_difficultyWindow = _difficultyWindow;
	
    if (_difficultyWindow > _timestamps.size()) {
		t_difficultyWindow = _timestamps.size();
    }
	
    std::vector<uint64_t> _timestamps_tmp(_timestamps.end() - t_difficultyWindow, _timestamps.end());
    std::vector<uint64_t> _cumulativeDifficulties_tmp(_cumulativeDifficulties.end() - t_difficultyWindow, _cumulativeDifficulties.end());

    size_t length = _timestamps_tmp.size();
	
    assert(length == _cumulativeDifficulties_tmp.size());
    assert(length <= _difficultyWindow);
	
    if (length <= 1) {
		return 1;
    }

    sort(_timestamps_tmp.begin(), _timestamps_tmp.end());
	
	size_t cutBegin, cutEnd;
	
//#2
    assert(2 * z_difficultyCut <= _difficultyWindow - 2);
	
    if (length <= _difficultyWindow - 2 * z_difficultyCut) {
		cutBegin = 0;
		cutEnd = length;
    } else {
		cutBegin = (length - (_difficultyWindow - 2 * z_difficultyCut) + 1) / 2;
		cutEnd = cutBegin + (_difficultyWindow - 2 * z_difficultyCut);
    }
//#3	
    assert(cutBegin + 2 <= cutEnd && cutEnd <= length);
	
    uint64_t timeSpan = _timestamps_tmp[cutEnd - 1] - _timestamps_tmp[cutBegin];
	
    if (timeSpan == 0) {
		timeSpan = 1;
    }
//#4
    difficulty_type totalWork = _cumulativeDifficulties_tmp[cutEnd - 1] - _cumulativeDifficulties_tmp[cutBegin];
    assert(totalWork > 0);

	uint64_t low, high;
	
    low = mul128(totalWork, m_difficultyTarget, &high);
//#5	
    if (high != 0 || std::numeric_limits<uint64_t>::max() - low < (timeSpan - 1)) {
		return 0;
    }
	
    uint64_t nextDiffZ = low / timeSpan;
//std::cout << khaki << "DIFF Z-" << _difficultyWindow << ": " << nextDiffZ << grey << std::endl;	
    return nextDiffZ;
}
//******************************************************************************************************************
//******************************************************************************************************************
difficulty_type Currency::nextDifficulty(
	std::vector<uint64_t> timestamps,
	std::vector<difficulty_type> cumulativeDifficulties, 
	size_t height) const {
//std::cout << std::endl;		
//std::cout << grey << "HEIGHT = " << height << std::endl;
	size_t ver = 1;

	if (parameters::DIFFICULTY_FNC_VER < 2 && height < parameters::DIFFICULTY_ZAWY_START_BLOCK) {
		ver = 1;
	}
	else {
		ver = 2;
	}
	
	if (parameters::DIFFICULTY_FNC_VER > 1) {
		ver = 2;
	}
	
	switch (ver) {
		case 1: {
			return nextDifficultyOld(timestamps, cumulativeDifficulties, height);
			break;
		}
		case 2: {
			return nextDifficultyZawy(timestamps, cumulativeDifficulties, parameters::DIFFICULTY_ZAWY_WINDOW);
			break;
		}
	}
	
	return 0;
}		
//******************************************************************************************************************
bool Currency::checkProofOfWorkV1(Crypto::cn_context& context, const Block& block, difficulty_type currentDiffic,
  Crypto::Hash& proofOfWork) const {
  if (block.majorVersion > BLOCK_MAJOR_VERSION_2) {
    return false;
  }

  if (!get_block_longhash(context, block, proofOfWork)) {
    return false;
  }

  return check_hash(proofOfWork, currentDiffic);
}

bool Currency::checkProofOfWorkV2(Crypto::cn_context& context, const Block& block, difficulty_type currentDiffic,
  Crypto::Hash& proofOfWork) const {
  if (block.majorVersion < BLOCK_MAJOR_VERSION_3) {
    return false;
  }

  if (!get_block_longhash(context, block, proofOfWork)) {
    return false;
  }

  if (!check_hash(proofOfWork, currentDiffic)) {
    return false;
  }
  

  TransactionExtraMergeMiningTag mmTag;
  if (!getMergeMiningTagFromExtra(block.rootBlock.baseTransaction.extra, mmTag)) {
    logger(ERROR) << "merge mining tag wasn't found in extra of the root block miner transaction";
    return false;
  }

  if (8 * sizeof(m_genesisBlockHash) < block.rootBlock.blockchainBranch.size()) {
    return false;
  }

  Crypto::Hash auxBlockHeaderHash;
  if (!get_aux_block_header_hash(block, auxBlockHeaderHash)) {
    return false;
  }

  Crypto::Hash auxBlocksMerkleRoot;
  Crypto::tree_hash_from_branch(block.rootBlock.blockchainBranch.data(), block.rootBlock.blockchainBranch.size(),
    auxBlockHeaderHash, &m_genesisBlockHash, auxBlocksMerkleRoot);

  if (auxBlocksMerkleRoot != mmTag.merkleRoot) {
    logger(ERROR, BRIGHT_YELLOW) << "Aux block hash wasn't found in merkle tree";
    return false;
  }

  return true;
}

bool Currency::checkProofOfWork(Crypto::cn_context& context, const Block& block, difficulty_type currentDiffic, Crypto::Hash& proofOfWork) const {
  switch (block.majorVersion) {
  case BLOCK_MAJOR_VERSION_1:
  case BLOCK_MAJOR_VERSION_2:
    return checkProofOfWorkV1(context, block, currentDiffic, proofOfWork);
  case BLOCK_MAJOR_VERSION_3:
  case BLOCK_MAJOR_VERSION_4:
    return checkProofOfWorkV2(context, block, currentDiffic, proofOfWork);
  }

  logger(ERROR, BRIGHT_RED) << "Unknown block major version: " << block.majorVersion << "." << block.minorVersion;
  return false;
}

size_t Currency::getApproximateMaximumInputCount(size_t transactionSize, size_t outputCount, size_t mixinCount) const {
  const size_t KEY_IMAGE_SIZE = sizeof(Crypto::KeyImage);
  const size_t OUTPUT_KEY_SIZE = sizeof(decltype(KeyOutput::key));
  const size_t AMOUNT_SIZE = sizeof(uint64_t) + 2; //varint
  const size_t GLOBAL_INDEXES_VECTOR_SIZE_SIZE = sizeof(uint8_t);//varint
  const size_t GLOBAL_INDEXES_INITIAL_VALUE_SIZE = sizeof(uint32_t);//varint
  const size_t GLOBAL_INDEXES_DIFFERENCE_SIZE = sizeof(uint32_t);//varint
  const size_t SIGNATURE_SIZE = sizeof(Crypto::Signature);
  const size_t EXTRA_TAG_SIZE = sizeof(uint8_t);
  const size_t INPUT_TAG_SIZE = sizeof(uint8_t);
  const size_t OUTPUT_TAG_SIZE = sizeof(uint8_t);
  const size_t PUBLIC_KEY_SIZE = sizeof(Crypto::PublicKey);
  const size_t TRANSACTION_VERSION_SIZE = sizeof(uint8_t);
  const size_t TRANSACTION_UNLOCK_TIME_SIZE = sizeof(uint64_t);

  const size_t outputsSize = outputCount * (OUTPUT_TAG_SIZE + OUTPUT_KEY_SIZE + AMOUNT_SIZE);
  const size_t headerSize = TRANSACTION_VERSION_SIZE + TRANSACTION_UNLOCK_TIME_SIZE + EXTRA_TAG_SIZE + PUBLIC_KEY_SIZE;
  const size_t inputSize = INPUT_TAG_SIZE + AMOUNT_SIZE + KEY_IMAGE_SIZE + SIGNATURE_SIZE + GLOBAL_INDEXES_VECTOR_SIZE_SIZE + GLOBAL_INDEXES_INITIAL_VALUE_SIZE +
                            mixinCount * (GLOBAL_INDEXES_DIFFERENCE_SIZE + SIGNATURE_SIZE);

  return (transactionSize - headerSize - outputsSize) / inputSize;
}

CurrencyBuilder::CurrencyBuilder(Logging::ILogger& log) : m_currency(log) {
  maxBlockNumber(parameters::CRYPTONOTE_MAX_BLOCK_NUMBER);
  maxBlockBlobSize(parameters::CRYPTONOTE_MAX_BLOCK_BLOB_SIZE);
  maxTxSize(parameters::CRYPTONOTE_MAX_TX_SIZE);
  publicAddressBase58Prefix(parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX);
  minedMoneyUnlockWindow(parameters::CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW);

  timestampCheckWindow(parameters::BLOCKCHAIN_TIMESTAMP_CHECK_WINDOW);
  blockFutureTimeLimit(parameters::CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT);

  moneySupply(parameters::MONEY_SUPPLY);

  rewardBlocksWindow(parameters::CRYPTONOTE_REWARD_BLOCKS_WINDOW);
  blockGrantedFullRewardZone(parameters::CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE);
  minerTxBlobReservedSize(parameters::CRYPTONOTE_COINBASE_BLOB_RESERVED_SIZE);

  numberOfDecimalPlaces(parameters::CRYPTONOTE_DISPLAY_DECIMAL_POINT);

  mininumFee(parameters::MINIMUM_FEE);
  defaultDustThreshold(parameters::DEFAULT_DUST_THRESHOLD);

  difficultyTarget(parameters::DIFFICULTY_TARGET);
  difficultyWindow(parameters::DIFFICULTY_WINDOW);
  difficultyLag(parameters::DIFFICULTY_LAG);
  difficultyCut(parameters::DIFFICULTY_CUT);

  depositMinAmount(parameters::DEPOSIT_MIN_AMOUNT);
  depositMinTerm(parameters::DEPOSIT_MIN_TERM);
  depositMaxTerm(parameters::DEPOSIT_MAX_TERM);
  depositMinTotalRateFactor(parameters::DEPOSIT_MIN_TOTAL_RATE_FACTOR);
  depositMaxTotalRate(parameters::DEPOSIT_MAX_TOTAL_RATE);

  maxBlockSizeInitial(parameters::MAX_BLOCK_SIZE_INITIAL);
  maxBlockSizeGrowthSpeedNumerator(parameters::MAX_BLOCK_SIZE_GROWTH_SPEED_NUMERATOR);
  maxBlockSizeGrowthSpeedDenominator(parameters::MAX_BLOCK_SIZE_GROWTH_SPEED_DENOMINATOR);

  lockedTxAllowedDeltaSeconds(parameters::CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_SECONDS);
  lockedTxAllowedDeltaBlocks(parameters::CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS);

  mempoolTxLiveTime(parameters::CRYPTONOTE_MEMPOOL_TX_LIVETIME);
  mempoolTxFromAltBlockLiveTime(parameters::CRYPTONOTE_MEMPOOL_TX_FROM_ALT_BLOCK_LIVETIME);
  numberOfPeriodsToForgetTxDeletedFromPool(parameters::CRYPTONOTE_NUMBER_OF_PERIODS_TO_FORGET_TX_DELETED_FROM_POOL);

  upgradeHeightV2(parameters::UPGRADE_HEIGHT_V2);
  upgradeHeightV3(parameters::UPGRADE_HEIGHT_V3);
  upgradeHeightV4(parameters::UPGRADE_HEIGHT_V4);
  upgradeVotingThreshold(parameters::UPGRADE_VOTING_THRESHOLD);
  upgradeVotingWindow(parameters::UPGRADE_VOTING_WINDOW);
  upgradeWindow(parameters::UPGRADE_WINDOW);

  fusionTxMaxSize(parameters::FUSION_TX_MAX_SIZE);
  fusionTxMinInputCount(parameters::FUSION_TX_MIN_INPUT_COUNT);
  fusionTxMinInOutCountRatio(parameters::FUSION_TX_MIN_IN_OUT_COUNT_RATIO);

  blocksFileName(parameters::CRYPTONOTE_BLOCKS_FILENAME);
  blocksCacheFileName(parameters::CRYPTONOTE_BLOCKSCACHE_FILENAME);
  blockIndexesFileName(parameters::CRYPTONOTE_BLOCKINDEXES_FILENAME);
  txPoolFileName(parameters::CRYPTONOTE_POOLDATA_FILENAME);
  blockchinIndicesFileName(parameters::CRYPTONOTE_BLOCKCHAIN_INDICES_FILENAME);

  testnet(false);
}
//////////////////////////////////////////////////
Transaction CurrencyBuilder::generateGenesisTransaction() 
{
	CryptoNote::Transaction tx;
	uint64_t prefix;
//	CryptoNote::AccountPublicAddress ac;	

	CryptoNote::AccountPublicAddress ac = boost::value_initialized<CryptoNote::AccountPublicAddress>();

//	if (!CryptoNote::parseAccountAddressString(prefix, ac, GENESIS_WALLET_Z))
//	{
//		std::cout << "Genesis address has wrong format" << std::endl;
//		ac = boost::value_initialized<CryptoNote::AccountPublicAddress>();
//		std::cout << "EMPTY Genesis address used" << std::endl;
//	};
	
	// height,medianSize,alreadyGeneratedCoins,currentBlockSize,fee,minerAddress,tx
	m_currency.constructMinerTx(0, 0, 0, 0, 0, ac, tx); // zero fee in genesis

	return tx;
}
/////////////////////////////////////////////////
CurrencyBuilder& CurrencyBuilder::numberOfDecimalPlaces(size_t val) {
  m_currency.m_numberOfDecimalPlaces = val;
  m_currency.m_coin = 1;
  for (size_t i = 0; i < m_currency.m_numberOfDecimalPlaces; ++i) {
    m_currency.m_coin *= 10;
  }

  return *this;
}

CurrencyBuilder& CurrencyBuilder::difficultyWindow(size_t val) {
  if (val < 2) {
    throw std::invalid_argument("val at difficultyWindow()");
  }
  m_currency.m_difficultyWindow = val;
  return *this;
}

CurrencyBuilder& CurrencyBuilder::upgradeVotingThreshold(unsigned int val) {
  if (val <= 0 || val > 100) {
    throw std::invalid_argument("val at upgradeVotingThreshold()");
  }
  m_currency.m_upgradeVotingThreshold = val;
  return *this;
}

CurrencyBuilder& CurrencyBuilder::upgradeWindow(size_t val) {
  if (val <= 0) {
    throw std::invalid_argument("val at upgradeWindow()");
  }
  m_currency.m_upgradeWindow = val;
  return *this;
}

}
