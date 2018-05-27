// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2014-2017 XDN-project developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// GOLDO
#pragma once

#include <cstdint>
#include <initializer_list>

namespace CryptoNote {
namespace parameters {

const uint64_t CRYPTONOTE_MAX_BLOCK_NUMBER                   = 500000000;
const size_t   CRYPTONOTE_MAX_BLOCK_BLOB_SIZE                = 500000000;
const size_t   CRYPTONOTE_MAX_TX_SIZE                        = 1000000000;
const uint64_t CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX       = 238;//Prefix of the wallet address: start with 135=GOLDO, 117=SLD
const size_t   CRYPTONOTE_MINED_MONEY_UNLOCK_WINDOW          = 18;// 4=GOLDO, 18=SLD
const uint64_t CRYPTONOTE_BLOCK_FUTURE_TIME_LIMIT            = 60 * 60 * 2;

const size_t   BLOCKCHAIN_TIMESTAMP_CHECK_WINDOW             = 30;

const uint64_t MONEY_SUPPLY                                  = static_cast<uint64_t>(-1);//Total amount of atomic units to be emitted

const size_t   CRYPTONOTE_REWARD_BLOCKS_WINDOW               = 100;
const size_t   CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE     = 1048576; //size of block (bytes) after which reward for block calculated using block size
const size_t   CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE_V1  = 1048576;  //size of block (bytes) after which reward for block calculated using block size
const size_t   CRYPTONOTE_COINBASE_BLOB_RESERVED_SIZE        = 600;
const size_t   CRYPTONOTE_DISPLAY_DECIMAL_POINT              = 8;//8=SLD, 4=GOLDO

const uint64_t COIN                                          = UINT64_C(100000000);// COIN - number of smallest units in one coin: 10000=GOLDO, 100000000=SLD
const uint64_t MINIMUM_FEE                                   = UINT64_C(1);// Transactions with less than this fee wouldn’t be accepted by daemons
const uint64_t DEFAULT_DUST_THRESHOLD                        = UINT64_C(2);//The amount bellow this value will be considered as dust: GOLDO=1, SLD=2

const uint64_t REAL_COINS_LIMITED_Z                          = UINT64_C(10000000);//Total amount of coins mineable with full reward: 10000000=SLD,10000000000=GOLDO
const uint64_t REAL_COINS_LOWSPEED_Z                         = UINT64_C(2500000);//Total amount of coins mineable with limited reward: 2500000=SLD,10000000000=GOLDO
const uint64_t MONEY_SUPPLY_MINEABLE_Z                       = REAL_COINS_LIMITED_Z*COIN;//Total amount of atomic units mineable with full reward
const uint64_t MONEY_SUPPLY_LOWSPEED_Z                       = REAL_COINS_LOWSPEED_Z*COIN;//Total amount of atomic units mineable with full reward
const size_t   RUSH_BLOCKS_Z                                 = 1000;//Premium rewarded blocks count: GOLDO=500, SLD=1000
const uint64_t RUSH_REWARD_Z                                 = (UINT64_C(100)*COIN);//Reward for a premium block: GOLDO=200000, SLD=100
const uint64_t COMMON_REWARD_Z                               = (UINT64_C(1)*COIN);//Reward for an ordinary block: GOLDO=1000, SLD=1
const uint64_t GENESIS_BLOCK_REWARD_Z                        = (UINT64_C(50000)*COIN);//Premine amount, in atomic units: SLD=50000, GOLDO=10.000.000
const size_t   RUSH_REWARD_LOCK_Z                            = 750000;//Premium rewarded blocks lock time: GOLDO=20000, SLD=750000

const uint64_t DIFFICULTY_TARGET                             = 20;//Difficulty target is an ideal time period between blocks, in seconds. 20=SLD, 720=GOLDO
const uint64_t EXPECTED_NUMBER_OF_BLOCKS_PER_DAY             = 24 * 60 * 60 / DIFFICULTY_TARGET;
const size_t   DIFFICULTY_WINDOW                             = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY/12; // blocks
const size_t   DIFFICULTY_CUT                                = 60;//Timestamps to cut after sorting
const size_t   DIFFICULTY_LAG                                = 20;//Lag of calculating the difficulty in terms of blocks

static_assert(2 * DIFFICULTY_CUT <= DIFFICULTY_WINDOW - 2, "Bad DIFFICULTY_WINDOW or DIFFICULTY_CUT");

const size_t   DIFFICULTY_FNC_VER                            = 0;//0=SLD, 2=GOLDO
const uint32_t DIFFICULTY_ZAWY_START_BLOCK                   = 24000;//24000=SLD, 1=GOLDO
const size_t   DIFFICULTY_ZAWY_WINDOW                        = 30;//30=SLD, 12=GOLDO

const uint64_t DEPOSIT_MIN_AMOUNT                            = 1 * COIN;
const uint32_t DEPOSIT_MIN_TERM                              = 11000;
const uint32_t DEPOSIT_MAX_TERM                              = 10 * 12 * 11000;
const uint64_t DEPOSIT_MIN_TOTAL_RATE_FACTOR                 = 77000;
const uint64_t DEPOSIT_MAX_TOTAL_RATE                        = 11;

static_assert(DEPOSIT_MIN_TERM > 0, "Bad DEPOSIT_MIN_TERM");
static_assert(DEPOSIT_MIN_TERM <= DEPOSIT_MAX_TERM, "Bad DEPOSIT_MAX_TERM");
static_assert(DEPOSIT_MIN_TERM * DEPOSIT_MAX_TOTAL_RATE > DEPOSIT_MIN_TOTAL_RATE_FACTOR, "Bad DEPOSIT_MIN_TOTAL_RATE_FACTOR or DEPOSIT_MAX_TOTAL_RATE");

const size_t   MAX_BLOCK_SIZE_INITIAL                        = CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE * 10;
const uint64_t MAX_BLOCK_SIZE_GROWTH_SPEED_NUMERATOR         = 100 * 1024;
const uint64_t MAX_BLOCK_SIZE_GROWTH_SPEED_DENOMINATOR       = 365 * 24 * 60 * 60 / DIFFICULTY_TARGET;

const uint64_t CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS     = 1;
const uint64_t CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_SECONDS    = DIFFICULTY_TARGET * CRYPTONOTE_LOCKED_TX_ALLOWED_DELTA_BLOCKS;

const uint64_t CRYPTONOTE_MEMPOOL_TX_LIVETIME                = (60 * 60 * 4); //seconds, 4 hours
const uint64_t CRYPTONOTE_MEMPOOL_TX_FROM_ALT_BLOCK_LIVETIME = (60 * 60 * 8); //seconds, 8 hours
const uint64_t CRYPTONOTE_NUMBER_OF_PERIODS_TO_FORGET_TX_DELETED_FROM_POOL = 7;  // CRYPTONOTE_NUMBER_OF_PERIODS_TO_FORGET_TX_DELETED_FROM_POOL * CRYPTONOTE_MEMPOOL_TX_LIVETIME = time to forget tx

const size_t   FUSION_TX_MAX_SIZE                            = CRYPTONOTE_BLOCK_GRANTED_FULL_REWARD_ZONE * 30 / 100;
const size_t   FUSION_TX_MIN_INPUT_COUNT                     = 12;
const size_t   FUSION_TX_MIN_IN_OUT_COUNT_RATIO              = 4;

const uint32_t UPGRADE_HEIGHT_V2                             = 2;
const uint32_t UPGRADE_HEIGHT_V3                             = 3;
const uint32_t UPGRADE_HEIGHT_V4                             = 4;
const unsigned UPGRADE_VOTING_THRESHOLD                      = 90;               // percent
const size_t   UPGRADE_VOTING_WINDOW                         = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY;  // blocks
const size_t   UPGRADE_WINDOW                                = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY;  // blocks
static_assert(0 < UPGRADE_VOTING_THRESHOLD && UPGRADE_VOTING_THRESHOLD <= 100, "Bad UPGRADE_VOTING_THRESHOLD");
static_assert(UPGRADE_VOTING_WINDOW > 1, "Bad UPGRADE_VOTING_WINDOW");

const char     CRYPTONOTE_BLOCKS_FILENAME[]                  = "chain.data";
const char     CRYPTONOTE_BLOCKINDEXES_FILENAME[]            = "idx.data";
const char     CRYPTONOTE_BLOCKSCACHE_FILENAME[]             = "cache.data";
const char     CRYPTONOTE_POOLDATA_FILENAME[]                = "mempool.data";
const char     P2P_NET_DATA_FILENAME[]                       = "p2p.data";
const char     CRYPTONOTE_BLOCKCHAIN_INDICES_FILENAME[]      = "chainidx.dat";
const char     MINER_CONFIG_FILE_NAME[]                      = "common.json";
} // parameters

const uint64_t TIMESTAMP_HACK_1_BLOCK_HEIGHT_Z               = 377777;//400000=SLD, 1=GOLDO
const uint64_t TIMESTAMP_HACK_1_FUTURE_TIME_LIMIT_Z          = 60 * 7;//420 seconds/21 block = SLD

const uint64_t START_BLOCK_REWARD                            = (UINT64_C(320000) * parameters::COIN);
const uint64_t MIN_BLOCK_REWARD                              = (UINT64_C(150) * parameters::COIN);
const uint64_t REWARD_HALVING_INTERVAL                       = (UINT64_C(11000));

const char     CRYPTONOTE_TICKER[]                           = "goldo";
const char     CRYPTONOTE_NAME[]                             = "Goldo";

const char GENESIS_COINBASE_TX_HEX[] = "010101ff00002101f0c11cb027ca12cb2b52d82bbe1851432ca1aabc5362375cddfb1c9606a8d135";

const uint32_t GENESIS_NONCE                                 = 24;
const char     GENESIS_WALLET_Z[]                            = "LaFwtcPWr5KhN5q1t5BYjxG6HqYph1x7B49AVdeWUewC1dMrFzXMuuJBo5YHQQRqQy7wRWQnctEcDY1eRQn76APPQycHeuj";

//CRYPTONOTE_NETWORK is defined @ \P2p\P2pNetworks.h
//CRYPTONOTE_NETWORK = 24d91a1b-3aa0-6ddf-7bab-e0b4fb48f9e9
//const static boost::uuids::uuid CRYPTONOTE_NETWORK = { { 0x24, 0xD9, 0x1A, 0x1B, 0x3A, 0xA0, 0x6D, 0xDF, 0x7B, 0xAB, 0xE0, 0xB4, 0xFB, 0x48, 0xF9, 0xE9 } };

const uint8_t  TRANSACTION_VERSION_1                         =  1;
const uint8_t  TRANSACTION_VERSION_2                         =  2;
const uint8_t  BLOCK_MAJOR_VERSION_1                         =  1;
const uint8_t  BLOCK_MAJOR_VERSION_2                         =  2;
const uint8_t  BLOCK_MAJOR_VERSION_3                         =  3;
const uint8_t  BLOCK_MAJOR_VERSION_4                         =  4;
const uint8_t  BLOCK_MINOR_VERSION_0                         =  0;
const uint8_t  BLOCK_MINOR_VERSION_1                         =  1;

const size_t   BLOCKS_IDS_SYNCHRONIZING_DEFAULT_COUNT        =  10000;  //by default, blocks ids count in synchronizing
const size_t   BLOCKS_SYNCHRONIZING_DEFAULT_COUNT            =  128;    //by default, blocks count in blocks downloading
const size_t   COMMAND_RPC_GET_BLOCKS_FAST_MAX_COUNT         =  1000;

const int      P2P_DEFAULT_PORT                              = 33721;//GOLDO=33721, SLD=33711
const int      RPC_DEFAULT_PORT                              = 33722;//GOLDO=33722, SLD=33712

const size_t   P2P_LOCAL_WHITE_PEERLIST_LIMIT                =  512;
const size_t   P2P_LOCAL_GRAY_PEERLIST_LIMIT                 =  2048;

const size_t   P2P_CONNECTION_MAX_WRITE_BUFFER_SIZE          = 64 * 1024 * 1024; // 64 MB
const uint32_t P2P_DEFAULT_CONNECTIONS_COUNT                 = 8;
const size_t   P2P_DEFAULT_WHITELIST_CONNECTIONS_PERCENT     = 70;
const uint32_t P2P_DEFAULT_HANDSHAKE_INTERVAL                = 300;           // seconds
const uint32_t P2P_DEFAULT_PACKET_MAX_SIZE                   = 50000000;      // 50000000 bytes maximum packet size
const uint32_t P2P_DEFAULT_PEERS_IN_HANDSHAKE                = 64;
const uint32_t P2P_DEFAULT_CONNECTION_TIMEOUT                = 5000;          // 5 seconds
const uint32_t P2P_DEFAULT_PING_CONNECTION_TIMEOUT           = 2000;          // 2 seconds
const uint64_t P2P_DEFAULT_INVOKE_TIMEOUT                    = 60 * 2 * 1000; // 2 minutes
const size_t   P2P_DEFAULT_HANDSHAKE_INVOKE_TIMEOUT          = 5000;          // 5 seconds
const char     P2P_STAT_TRUSTED_PUB_KEY[]                    = "2a0b780aa4dc2f95cf4251c5c1cdab5205e75edef907e74de6226df2a5778ea5";

const std::initializer_list<const char*> SEED_NODES = {
	"s1.soldo.in:33721",  
	"s2.soldo.in:33721",  
	"s3.soldo.in:33721",  
	"s4.soldo.in:33721",  
	"s5.soldo.in:33721",  
	"s6.soldo.in:33721",  
};

struct CheckpointData {
  uint32_t height;
  const char* blockId;
};

#ifdef __GNUC__
__attribute__((unused))
#endif

// You may add here other checkpoints using the following format:
// {<block height>, "<block hash>"},
const std::initializer_list<CheckpointData> CHECKPOINTS = {
	{ 10,       "9d9c8d35abe3df6d6b568ceaec0484ab4de90b117f449e3f20268d4c5157cd3c" },
	{ 500,      "6484b98d348a8bb912ccac1f57bf489e79b2e7f182fbcd85f9e6fb6cb9b75020" },
	{ 1000,     "e71a72d72446b61b70fb11bc4b9f5a9b4acbea772522876181aaab6cfeb9f8b3" },
	{ 5000,     "8c25c2783910ede539fa575edf7dbd8aae87e851081a163f51f4af381a3dc488" },
	{ 10000,    "9f5a62a04e0dea35f922c9c46240bc26d766c3cf9d45df62cc1dcb3a2b296042" },
	{ 20000,    "338b94eea1c836545b9ac52635bf56e793c4d7087327b8344f801d19d244efdc" },
	{ 24000,    "e191d6292e928410481ed6209c4d492e0f0eff82723f9a18031f85d3955dffc9" },
	{ 50000,    "d77bbc550a5e08502e8bd58e6cd5f0ec24ab00179135cd4eb99a9d7c74d8cc4e" },
	{ 75000,    "75a833470bff666bc414a5803ecd9bf1a7c19b75049ed6c4d5903690210f66fd" },
	{ 100000,   "2a9f74669832fe2e2a14129d4df423d8d2fa1547a0a24358a51fc71b810ff641" },
	{ 125000,   "776a8b2b40cf5297375b5813fde5480c6ae8525db35fad73b7b523d6d67771c8" },
	{ 150000,   "b8d953e0b5d81f8a2b0db3d3881107ec676d72e0368c88c25085a913830a6656" },
	{ 175000,   "6c8b5543f94a9fbc3914fdfd0a58277d8ff1879b1f173db6890f99425fecc3aa" },
	{ 200000,   "b529eb8efbd46d978d7ce48f18ab07bb1bf6d0e4ddb85adf9808aed781fc045b" },
	{ 225000,   "6ceb7e3443978adb2356608417bb3dd61184246979d42ad8131ab274b2138a18" },
	{ 250000,   "9269ae2275b997f5d5d34e14a40c89471cf0f5b127693cdb0ea9b4a58420d966" },
	{ 275000,   "94fb70d53ce490f44da2d15b5df8f7c498c721504c01c30e4cb6c58df26dafc7" },
	{ 300000,   "a2e254ad0064addff57808135d42ed84a42c89df93f4f3a7765d0bf0efa9b8fe" },
	{ 325000,   "a012f24b7f2a9d417a4e79a0634212aba806ec13fd78a9990e36f7339a1c0a7d" },
	{ 350000,   "aecc323b82167e13b6d0c26493af87f9b97fbbc6a5cc0e93ad46a388d0889ac0" },
	{ 377777,   "fda74af4c706dd90c3b1c229e844cb95c2f6247f412e90971355f41257599172" },
	{ 400000,   "9a7d7ac7c224ff4f9c0d7c5e8a3a749934b59747ddac04292253adb4b7f88f90" },
	{ 425000,   "a4d8679422aee196f51126bde457402b031f9abce3688980f5239b85ad515296" },
	{ 450000,   "bfb507c5057d273eb3b4a256a8e729f6e18de4b4cd4de9326de030b7cf30ad14" },
	{ 475000,   "76e9f6a822e3e3a9078132ce8b104701b1a9c55c990c4d2185631802da0b5fbe" },
	{ 500000,   "4569f0c235536786cc0d2057e1a8b9a857ed0d97e03542cc86fe4ba5756678eb" },
	{ 525000,   "eec5f77dd46dacc95ee1bf78c939bb6caef15f1a1614b5678cc288ccfbf7a7ab" },
	{ 550000,   "b920182434221a65d64726ce62bc9d9640cd7ad0e7358d1e1f8b6d02770e9813" },
	{ 575000,   "2d012a3dc3368e9083a8cdc8a21e2f4e384aaac2a6ca1fcd0bb3cf175a5d0ae8" },
	{ 600000,   "9f7306ae82b72b8c57a97a7a422825068c0187fc6f0f8aa4e067242d66f1de94" },
	{ 625000,   "465d13678c2bfe699ae4baf58a0e3c8b698b73d08cac548a6ed6df5bb38de8c2" },
	{ 650000,   "5a94c7f078099866ecaf2f8512692be48b1bff63685409e17ab1c09b3d701223" },
	{ 675000,   "c35bd4f1b8ac5f0c876855c28d463d39bea9721ca651d55f1dad32b7ac37a228" },
	{ 700000,   "f04a589aba3ab4104fd29d0aa1429e5e28ae25c63f90b7eefc489433dd6c6a6b" },
	{ 717969,   "52f7aead5f5d924ff7c262a630c635b93b0c97a980700c1e22d6113d8c0d1625" },
	{ 725000,   "3de2a571a4c53ec9b47731399b98442c20d5f8f04902990b1e551ad8b4f932b9" },
	{ 750000,   "1b42efa22e47ce8980f33cdbc8a1ab823b3377bd261e2aa10e448f78d082cba3" },

};

} // CryptoNote

#define ALLOW_DEBUG_COMMANDS
