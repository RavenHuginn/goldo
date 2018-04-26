// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2014-2017 XDN-project developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SimpleWallet.h"

#include <ctime>
#include <fstream>
#include <future>
#include <iomanip>
#include <thread>
#include <set>
#include <sstream>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "Common/CommandLine.h"
#include "Common/SignalHandler.h"
#include "Common/StringTools.h"
#include "Common/PathTools.h"
#include "Common/Util.h"

#include <Common/Base58.h>

#include "CryptoNoteCore/CryptoNoteFormatUtils.h"
#include "CryptoNoteProtocol/CryptoNoteProtocolHandler.h"
#include "NodeRpcProxy/NodeRpcProxy.h"
#include "Rpc/CoreRpcServerCommandsDefinitions.h"
#include "Rpc/HttpClient.h"

#include "Wallet/WalletRpcServer.h"
#include "WalletLegacy/WalletLegacy.h"
#include "Wallet/LegacyKeysImporter.h"
#include "WalletLegacy/WalletHelper.h"

#include "version.h"

#include <Logging/LoggerManager.h>

#include "Common/ConsoleTools.h"
#include "zrainbow.h"
#include "ztime.h"

#if defined(WIN32)
#include <crtdbg.h>
#endif

using namespace CryptoNote;
using namespace Logging;
using Common::JsonValue;

namespace po = boost::program_options;

#define EXTENDED_LOGS_FILE "wallet_details.log"
#undef ERROR

namespace {

const command_line::arg_descriptor<std::string> arg_wallet_file = { "wallet-file", "Use wallet <arg>", "" };
const command_line::arg_descriptor<std::string> arg_generate_new_wallet = { "generate-new-wallet", "Generate new wallet and save it to <arg>", "" };
const command_line::arg_descriptor<std::string> arg_daemon_address = { "daemon-address", "Use daemon instance at <host>:<port>", "" };
const command_line::arg_descriptor<std::string> arg_daemon_host = { "daemon-host", "Use daemon instance at host <arg> instead of localhost", "" };
const command_line::arg_descriptor<std::string> arg_password = { "password", "Wallet password", "", true };
const command_line::arg_descriptor<uint16_t> arg_daemon_port = { "daemon-port", "Use daemon instance at port <arg> instead of 33711", 0 };
const command_line::arg_descriptor<uint32_t> arg_log_level = { "set_log", "", INFO, true };
const command_line::arg_descriptor<bool> arg_testnet = { "testnet", "Used to deploy test nets. The daemon must be launched with --testnet flag", false };
const command_line::arg_descriptor< std::vector<std::string> > arg_command = { "command", "" };
//$$$$
const command_line::arg_descriptor<bool> arg_classic = {"classic", "creates CLASSIC (old-style) wallet", false};
const command_line::arg_descriptor<bool> arg_restore_seed = {"restore-seed", "Recover wallet using mnemoseed", false};
const command_line::arg_descriptor<bool> arg_restore_keys = {"restore-keys", "Recover wallet using hexadecimal wallet keys", false};
const command_line::arg_descriptor<bool> arg_restore_legacy = {"restore-legacy", "Recover wallet using old-style recovery key", false};
const command_line::arg_descriptor<std::string> arg_seed = {"seed", "Specify Mnemoseed for wallet recovery", ""};
const command_line::arg_descriptor<std::string> arg_spend_key = {"spend-key", "Specify Private Spend Key for wallet recovery", ""};
const command_line::arg_descriptor<std::string> arg_view_key = {"view-key", "Specify Private View Key for wallet recovery", ""};
const command_line::arg_descriptor<std::string> arg_legacy_key = {"legacy-key", "Specify old-style Recovery Key for wallet recovery", ""};
//$$$$
bool parseUrlAddress(const std::string& url, std::string& address, uint16_t& port) {
  auto pos = url.find("://");
  size_t addrStart = 0;

  if (pos != std::string::npos) {
    addrStart = pos + 3;
  }

  auto addrEnd = url.find(':', addrStart);

  if (addrEnd != std::string::npos) {
    auto portEnd = url.find('/', addrEnd);
    port = Common::fromString<uint16_t>(url.substr(
      addrEnd + 1, portEnd == std::string::npos ? std::string::npos : portEnd - addrEnd - 1));
  } else {
    addrEnd = url.find('/');
    port = 80;
  }

  address = url.substr(addrStart, addrEnd - addrStart);
  return true;
}


inline std::string interpret_rpc_response(bool ok, const std::string& status) {
  std::string err;
  if (ok) {
    if (status == CORE_RPC_STATUS_BUSY) {
      err = "daemon is busy. Please try later";
    } else if (status != CORE_RPC_STATUS_OK) {
      err = status;
    }
  } else {
    err = "possible lost connection to daemon";
  }
  return err;
}

template <typename IterT, typename ValueT = typename IterT::value_type>
class ArgumentReader {
public:

  ArgumentReader(IterT begin, IterT end) :
    m_begin(begin), m_end(end), m_cur(begin) {
  }

  bool eof() const {
    return m_cur == m_end;
  }

  ValueT next() {
    if (eof()) {
      throw std::runtime_error("unexpected end of arguments");
    }

    return *m_cur++;
  }

private:

  IterT m_cur;
  IterT m_begin;
  IterT m_end;
};

struct TransferCommand {
  const CryptoNote::Currency& m_currency;
  size_t fake_outs_count;
  std::vector<CryptoNote::WalletLegacyTransfer> dsts;
  std::vector<uint8_t> extra;
  uint64_t fee;
  std::map<std::string, std::vector<WalletLegacyTransfer>> aliases;
  std::vector<std::string> messages;
  uint64_t ttl;

  TransferCommand(const CryptoNote::Currency& currency) :
    m_currency(currency), fake_outs_count(0), fee(currency.minimumFee()), ttl(0) {
  }

  bool parseArguments(LoggerRef& logger, const std::vector<std::string> &args) {

    ArgumentReader<std::vector<std::string>::const_iterator> ar(args.begin(), args.end());

    try {

      auto mixin_str = ar.next();

      if (!Common::fromString(mixin_str, fake_outs_count)) {
        logger(ERROR, BRIGHT_RED) << "mixin_count should be non-negative integer, got " << mixin_str;
        return false;
      }

      bool feeFound = false;
      bool ttlFound = false;
      while (!ar.eof()) {

        auto arg = ar.next();

        if (arg.size() && arg[0] == '-') {

          const auto& value = ar.next();

          if (arg == "-p") {
            if (!createTxExtraWithPaymentId(value, extra)) {
              logger(ERROR, BRIGHT_RED) << "payment ID has invalid format: \"" << value << "\", expected 64-character string";
              return false;
            }
          } else if (arg == "-f") {
            feeFound = true;

            if (ttlFound) {
              logger(ERROR, BRIGHT_RED) << "Transaction with TTL can not have fee";
              return false;
            }

            bool ok = m_currency.parseAmount(value, fee);
            if (!ok) {
              logger(ERROR, BRIGHT_RED) << "Fee value is invalid: " << value;
              return false;
            }

            if (fee < m_currency.minimumFee()) {
              logger(ERROR, BRIGHT_RED) << "Fee value is less than minimum: " << m_currency.minimumFee();
              return false;
            }
          } else if (arg == "-m") {
            messages.emplace_back(value);
          } else if (arg == "-ttl") {
            ttlFound = true;

            if (feeFound) {
              logger(ERROR, BRIGHT_RED) << "Transaction with fee can not have TTL";
              return false;
            } else {
              fee = 0;
            }

            if (!Common::fromString(value, ttl) || ttl < 1 || ttl * 60 > m_currency.mempoolTxLiveTime()) {
              logger(ERROR, BRIGHT_RED) << "TTL has invalid format: \"" << value << "\", " <<
                "enter time from 1 to " << (m_currency.mempoolTxLiveTime() / 60) << " minutes";
              return false;
            }
          }
        } else {
          WalletLegacyTransfer destination;
          CryptoNote::TransactionDestinationEntry de;
          std::string aliasUrl;

          if (!m_currency.parseAccountAddressString(arg, de.addr)) {
            aliasUrl = arg;
          }

          auto value = ar.next();
          bool ok = m_currency.parseAmount(value, de.amount);
          if (!ok || 0 == de.amount) {
            logger(ERROR, BRIGHT_RED) << "amount is wrong: " << arg << ' ' << value <<
              ", expected number from 0 to " << m_currency.formatAmount(std::numeric_limits<uint64_t>::max());
            return false;
          }

          if (aliasUrl.empty()) {
            destination.address = arg;
            destination.amount = de.amount;
            dsts.push_back(destination);
          } else {
            aliases[aliasUrl].emplace_back(WalletLegacyTransfer{"", static_cast<int64_t>(de.amount)});
          }
        }
      }

      if (dsts.empty() && aliases.empty()) {
        logger(ERROR, BRIGHT_RED) << "At least one destination address is required";
        return false;
      }
    } catch (const std::exception& e) {
      logger(ERROR, BRIGHT_RED) << e.what();
      return false;
    }

    return true;
  }
};

JsonValue buildLoggerConfiguration(Level level, const std::string& logfile) {
  JsonValue loggerConfiguration(JsonValue::OBJECT);
  loggerConfiguration.insert("globalLevel", static_cast<int64_t>(level));

  JsonValue& cfgLoggers = loggerConfiguration.insert("loggers", JsonValue::ARRAY);

  JsonValue& consoleLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
  consoleLogger.insert("type", "console");
  consoleLogger.insert("level", static_cast<int64_t>(TRACE));
  consoleLogger.insert("pattern", "");

  JsonValue& fileLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
  fileLogger.insert("type", "file");
  fileLogger.insert("filename", logfile);
  fileLogger.insert("level", static_cast<int64_t>(TRACE));

  return loggerConfiguration;
}
std::error_code initAndLoadWallet(IWalletLegacy& wallet, std::istream& walletFile, const std::string& password) {
  WalletHelper::InitWalletResultObserver initObserver;
  std::future<std::error_code> f_initError = initObserver.initResult.get_future();

  WalletHelper::IWalletRemoveObserverGuard removeGuard(wallet, initObserver);
  wallet.initAndLoad(walletFile, password);
  auto initError = f_initError.get();

  return initError;
}
///////////////////////////////////////////////////////////////////////////////
std::string tryToOpenWalletOrLoadKeysOrThrow(LoggerRef& logger, std::unique_ptr<IWalletLegacy>& wallet, const std::string& walletFile, const std::string& password) {
  std::string keys_file, walletFileName;
  WalletHelper::prepareFileNames(walletFile, keys_file, walletFileName);

  boost::system::error_code ignore;
  bool keysExists = boost::filesystem::exists(keys_file, ignore);
  bool walletExists = boost::filesystem::exists(walletFileName, ignore);
  if (!walletExists && !keysExists && boost::filesystem::exists(walletFile, ignore)) {
    boost::system::error_code renameEc;
    boost::filesystem::rename(walletFile, walletFileName, renameEc);
    if (renameEc) {
      throw std::runtime_error("failed to rename file '" + walletFile + "' to '" + walletFileName + "': " + renameEc.message());
    }

    walletExists = true;
  }

  if (walletExists) {
    logger(INFO,YELLOW) << "Loading wallet...";
    std::ifstream walletFile;
    walletFile.open(walletFileName, std::ios_base::binary | std::ios_base::in);
    if (walletFile.fail()) {
      throw std::runtime_error("error opening wallet file '" + walletFileName + "'");
    }

    auto initError = initAndLoadWallet(*wallet, walletFile, password);

    walletFile.close();
    if (initError) { //bad password, or legacy format
      if (keysExists) {
        std::stringstream ss;
        CryptoNote::importLegacyKeys(keys_file, password, ss);
        boost::filesystem::rename(keys_file, keys_file + ".back");
        boost::filesystem::rename(walletFileName, walletFileName + ".back");

        initError = initAndLoadWallet(*wallet, ss, password);
        if (initError) {
          throw std::runtime_error("failed to load wallet: " + initError.message());
        }

        logger(INFO) << "Storing wallet...";

        try {
          CryptoNote::WalletHelper::storeWallet(*wallet, walletFileName);
        } catch (std::exception& e) {
          logger(ERROR, BRIGHT_RED) << "Failed to store wallet: " << e.what();
          throw std::runtime_error("error saving wallet file '" + walletFileName + "'");
        }

        logger(INFO, BRIGHT_GREEN) << "Stored ok";
        return walletFileName;
      } else { // no keys, wallet error loading
        throw std::runtime_error("can't load wallet file '" + walletFileName + "', check password");
      }
    } else { //new wallet ok 
      return walletFileName;
    }
  } else if (keysExists) { //wallet not exists but keys presented
    std::stringstream ss;
    CryptoNote::importLegacyKeys(keys_file, password, ss);
    boost::filesystem::rename(keys_file, keys_file + ".back");

    WalletHelper::InitWalletResultObserver initObserver;
    std::future<std::error_code> f_initError = initObserver.initResult.get_future();

    WalletHelper::IWalletRemoveObserverGuard removeGuard(*wallet, initObserver);
    wallet->initAndLoad(ss, password);
    auto initError = f_initError.get();

    removeGuard.removeObserver();
    if (initError) {
      throw std::runtime_error("failed to load wallet: " + initError.message());
    }

    logger(INFO) << "Storing wallet...";

    try {
      CryptoNote::WalletHelper::storeWallet(*wallet, walletFileName);
    } catch(std::exception& e) {
      logger(ERROR, BRIGHT_RED) << "Failed to store wallet: " << e.what();
      throw std::runtime_error("error saving wallet file '" + walletFileName + "'");
    }

    logger(INFO, BRIGHT_GREEN) << "Stored ok";
    return walletFileName;
  } else { //no wallet no keys
    throw std::runtime_error("wallet file '" + walletFileName + "' is not found");
  }
}

std::string makeCenteredString(size_t width, const std::string& text) {
  if (text.size() >= width) {
    return text;
  }

  size_t offset = (width - text.size() + 1) / 2;
  return std::string(offset, ' ') + text + std::string(width - text.size() - offset, ' ');
}

const size_t TIMESTAMP_MAX_WIDTH = 19;
const size_t HASH_MAX_WIDTH = 64;
const size_t TOTAL_AMOUNT_MAX_WIDTH = 20;
const size_t FEE_MAX_WIDTH = 14;
const size_t BLOCK_MAX_WIDTH = 7;
const size_t UNLOCK_TIME_MAX_WIDTH = 11;
/*
///////////////////////////////////////////////////////////////////////////////
std::string get_now_time_str() {
	char timeString[20];
	time_t now = time(0);
	struct tm tstruct;
    tstruct = *localtime(&now);
	
	if (std::strftime(timeString, sizeof(timeString), "%Y-%m-%d-%H-%M-%S", &tstruct) == 0) 
		{
			throw std::runtime_error("time buffer is too small");
		}
	return std::string(timeString);
}
///////////////////////////////////////////////////////////////////////////////
*/
// get_tx_time_str(txInfo)
std::string get_tx_time_str(const WalletLegacyTransaction& txInfo) {
	char timeString[20];
	time_t timestamp = static_cast<time_t>(txInfo.timestamp);

	if (std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp)) == 0) 
		{
			throw std::runtime_error("time buffer is too small");
		}
	return std::string(timeString);
}
///////////////////////////////////////////////////////////////////////////////
void print_keys_to_stream(const AccountKeys keys, const std::string _s, bool _is_to_file, std::ostream& _stream) {
	
	std::string lend = "";
	
	if (_is_to_file) {
		lend = "\r\n";
	}

	_stream << lend << std::endl << yellow << _s << lend << std::endl << lend << std::endl
		<< maroon << Common::podToHex(keys.spendSecretKey) << red << " - PRIVATE spend key" << lend << std::endl
		<< purple << Common::podToHex(keys.viewSecretKey) << magenta << " - PRIVATE view key" << lend << std::endl
		<< green << Common::podToHex(keys.address.spendPublicKey) << lime << " - PUBLIC spend key" << lend << std::endl
		<< teal << Common::podToHex(keys.address.viewPublicKey) << cyan << " - PUBLIC view key" << lend << std::endl << lend << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
void printListTransfersHeader(LoggerRef& logger) {
  std::string header = makeCenteredString(TIMESTAMP_MAX_WIDTH, "timestamp") + "  ";
  header += makeCenteredString(HASH_MAX_WIDTH, "hash") + "  ";
  header += makeCenteredString(TOTAL_AMOUNT_MAX_WIDTH, "total amount") + "  ";
  header += makeCenteredString(FEE_MAX_WIDTH, "fee") + "  ";
  header += makeCenteredString(BLOCK_MAX_WIDTH, "block") + "  ";
  header += makeCenteredString(UNLOCK_TIME_MAX_WIDTH, "unlock time");

  std::cout << header << std::endl << std::string(header.size(), '_') << std::endl << std::endl << grey; 
}
//////////////////////////////////////////////////////////////////////////
void printListTransfersItem(LoggerRef& logger, const WalletLegacyTransaction& txInfo, IWalletLegacy& wallet, const Currency& currency) {
  std::vector<uint8_t> extraVec = Common::asBinaryArray(txInfo.extra);

  Crypto::Hash paymentId;
  std::string paymentIdStr = (getPaymentIdFromTxExtra(extraVec, paymentId) && paymentId != NULL_HASH ? Common::podToHex(paymentId) : "");

	if (txInfo.totalAmount >= 0) 
		{
			std::cout << green;
		} 
	else 
		{
			std::cout << maroon;
		}
  
	std::cout
		<< std::setw(TIMESTAMP_MAX_WIDTH) << get_tx_time_str(txInfo)
		<< "  " << std::setw(HASH_MAX_WIDTH) << Common::podToHex(txInfo.hash)
		<< "  " << std::setw(TOTAL_AMOUNT_MAX_WIDTH) << currency.formatAmount(txInfo.totalAmount)
		<< "  " << std::setw(FEE_MAX_WIDTH) << currency.formatAmount(txInfo.fee)
		<< "  " << std::setw(BLOCK_MAX_WIDTH) << txInfo.blockHeight
		<< "  " << std::setw(UNLOCK_TIME_MAX_WIDTH) << txInfo.unlockTime << std::endl << grey;
	
	if (!paymentIdStr.empty()) 
		{
			std::cout << blue << " PAYMENT ID: " << paymentIdStr << std::endl;
		}

  if (txInfo.totalAmount < 0) 
  {
    if (txInfo.transferCount > 0) 
	{
      std::cout << purple << " TRANSFERS >>" << std::endl;
      for (TransferId id = txInfo.firstTransferId; id < txInfo.firstTransferId + txInfo.transferCount; ++id) 
	  {
        WalletLegacyTransfer tr;
        wallet.getTransfer(id, tr);
        std::cout << magenta << "  " << tr.address << " = " << currency.formatAmount(tr.amount)<< std::endl << grey;
      }
    }
  }
//$$$$$$
	if (txInfo.messages.size() > 0) 
	{
		std::cout << teal  << " COMMENTS >>" << std::endl;
		for(int i = 0; i < txInfo.messages.size(); i++)
			std::cout << cyan << "  " << txInfo.messages[i] << std::endl << grey;
	}
//$$$$$$
}
//////////////////////////////////////////////////////////////////////////
void printTxComments(const WalletLegacyTransaction& txInfo) 
{
	std::vector<uint8_t> extraVec = Common::asBinaryArray(txInfo.extra);

	if (txInfo.messages.size() > 0) 
	{
		std::cout  << teal << Common::podToHex(txInfo.hash) << " @ " << get_tx_time_str(txInfo) << " >> " << txInfo.blockHeight << std::endl << grey;;
		for(int i = 0; i < txInfo.messages.size(); i++)
			std::cout << cyan << "  " << txInfo.messages[i] << std::endl << grey;;
	}
}
///////////////////////////////////////////////////////////////////////////////
std::string prepareWalletBackupFilename(const std::string& walletBaseName) 
{
	return walletBaseName + ".backup.txt";
}
///////////////////////////////////////////////////////////////////////////////
bool writeBackupFile
	(
	const std::string& backupFilename, 
	const std::string& address,
	const std::string& wall,
	const std::string& pass,
	const AccountKeys keys,
	bool success,
	const std::string& electrum_words
	) 
{
	std::ofstream backupFile(backupFilename, std::ios::out | std::ios::trunc | std::ios::binary);
	
	if (!backupFile.good()) 
	{
		return false;
	}
	
	backupFile << "Print this file or save at removable media\r\n";
	backupFile << "YOU MUST KEEP THIS DATA TOP SECRET OR YOUR FUNDS COULD BE STOLEN\r\n";
	backupFile << "-------------------------------------------------------------\r\n";
	backupFile << 
		wall << " - wallet file\r\n" << 
		pass << " - password\r\n" << 
		address << " - address\r\n\r\n";
		
	print_keys_to_stream(keys, "Keys are: ", 1, backupFile);

	if (success) {
		backupFile << "\r\nThe wallet is MODERN, it has the recovery mnemonic seed: " << electrum_words << "\r\n";		
	}
	else {
		backupFile << "\r\nThe wallet is CLASSIC.\r\n";
	}	
	
	backupFile << "-------------------------------------------------------------\r\n";

	return true;
}
///////////////////////////////////////////////////////////////////////////////

bool writeAddressFile(const std::string& addressFilename, const std::string& address) {
  std::ofstream addressFile(addressFilename, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!addressFile.good()) {
    return false;
  }

  addressFile << address;

  return true;
}
////////////////////////////////
bool processServerAliasResponse(const std::string& response, std::string& address) {
  try {
    std::stringstream stream(response);
    JsonValue json;
    stream >> json;

    auto rootIt = json.getObject().find("digitalname1");
    if (rootIt == json.getObject().end()) {
      return false;
    }

    if (!rootIt->second.isArray()) {
      return false;
    }

    if (rootIt->second.size() == 0) {
      return false;
    }

    if (!rootIt->second[0].isObject()) {
      return false;
    }

    auto xdnIt = rootIt->second[0].getObject().find("sld");
    if (xdnIt == rootIt->second[0].getObject().end()) {
      return false;
    }

    address = xdnIt->second.getString();
  } catch (std::exception&) {
    return false;
  }

  return true;
}

bool splitUrlToHostAndUri(const std::string& aliasUrl, std::string& host, std::string& uri) {
  size_t protoBegin = aliasUrl.find("http://");
  if (protoBegin != 0 && protoBegin != std::string::npos) {
    return false;
  }

  size_t hostBegin = protoBegin == std::string::npos ? 0 : 7; //strlen("http://")
  size_t hostEnd = aliasUrl.find('/', hostBegin);

  if (hostEnd == std::string::npos) {
    uri = "/";
    host = aliasUrl.substr(hostBegin);
  } else {
    uri = aliasUrl.substr(hostEnd);
    host = aliasUrl.substr(hostBegin, hostEnd - hostBegin);
  }

  return true;
}

bool askAliasesTransfersConfirmation(const std::map<std::string, std::vector<WalletLegacyTransfer>>& aliases, const Currency& currency) {
  std::cout << "Would you like to send money to the following addresses?" << std::endl;

  for (const auto& kv: aliases) {
    for (const auto& transfer: kv.second) {
      std::cout << transfer.address << " " << std::setw(21) << currency.formatAmount(transfer.amount) << "  " << kv.first << std::endl;
    }
  }

  std::string answer;
  do {
    std::cout << "y/n: ";
    std::getline(std::cin, answer);
  } while (answer != "y" && answer != "Y" && answer != "n" && answer != "N");

  return answer == "y" || answer == "Y";
}

}
///////////////////////////////////////////////////////////////////////////////
bool simple_wallet::print_paper_wallet_to_stream(bool _is_to_file, std::ostream& _stream) {
	
	std::string lend = "";
	
	if (_is_to_file) {
		lend = "\r\n";
	}
	
	std::string electrum_words;
	std::string password = "*****";

	try {
		std::unique_ptr<CryptoNote::IWalletLegacy> sunduk;
		Crypto::SecretKey rkey;
		AccountKeys skeys;
		
		sunduk.reset(new WalletLegacy(m_currency, *m_node.get()));
		sunduk->initAndGenerateOrRecover(password,rkey,rkey,0,0,1);
		
		sunduk->getAccountKeys(skeys);
		bool success = try_seed(electrum_words,skeys);

		_stream << std::endl << yellow <<  "Paper wallet ADDRESS is: " << lend << lend << std::endl << std::endl << grey; 	
		_stream << m_wallet->getAddress()<< std::endl << lend << std::endl << grey; 
		
		print_keys_to_stream(skeys, "Paper wallet KEYS are: ", _is_to_file, _stream);

		if (success) 
		{
			_stream << yellow << "Paper wallet SEED is: "
				<< lend << std::endl << lend << std::endl << teal << electrum_words << lend << lend << std::endl << std::endl;      
		}
		else
		{
			_stream << std::endl << red 
				<< "The wallet is CLASSIC, no seed." << lend << lend << std::endl << std::endl;
		}
	}
	catch (const std::exception& e) {
		fail_msg_writer() << "failed to generate paper wallet: " << e.what();
		return false;
	}		
		
	return true;
}
///////////////////////////////////////////////////////////////////////////////
bool simple_wallet::print_paper_wallet(const std::string _file_name) {
		
	if (!_file_name.empty()) {
	
		std::ofstream paperFile(_file_name, std::ios::out | std::ios::trunc | std::ios::binary);
		
		if (!paperFile.good()) 
		{
			return false;
		}
		
		return print_paper_wallet_to_stream(1,paperFile);
	}
	else {
		return print_paper_wallet_to_stream(0,std::cout);
	}
}
///////////////////////////////////////////////////////////////////////////////
std::string simple_wallet::get_commands_str() {
  std::stringstream ss;
  ss << "Commands: " << ENDL;
  std::string usage = m_consoleHandler.getUsage();
  boost::replace_all(usage, "\n", "\n  ");
  usage.insert(0, "  ");
  ss << usage << ENDL;
  return ss.str();
}

bool simple_wallet::help(const std::vector<std::string> &args/* = std::vector<std::string>()*/) {
  success_msg_writer() << get_commands_str();
  return true;
}

bool simple_wallet::exit(const std::vector<std::string> &args) {
  m_consoleHandler.requestStop();
  return true;
}

simple_wallet::simple_wallet(System::Dispatcher& dispatcher, const CryptoNote::Currency& currency, Logging::LoggerManager& log) :
  m_dispatcher(dispatcher),
  m_daemon_port(0), 
  m_currency(currency), 
  logManager(log),
  logger(log, "simplewallet"),
  m_refresh_progress_reporter(*this), 
  m_initResultPromise(nullptr),
  m_walletSynchronized(false) {
  m_consoleHandler.setHandler("start_mining", boost::bind(&simple_wallet::start_mining, this, _1), "start_mining [<number_of_threads>] - Start mining in daemon");
  m_consoleHandler.setHandler("stop_mining", boost::bind(&simple_wallet::stop_mining, this, _1), "Stop mining in daemon");
  //m_consoleHandler.setHandler("refresh", boost::bind(&simple_wallet::refresh, this, _1), "Resynchronize transactions and balance");
  m_consoleHandler.setHandler("balance", boost::bind(&simple_wallet::show_balance, this, _1), "Show current wallet's balance");
  m_consoleHandler.setHandler("incoming", boost::bind(&simple_wallet::show_incoming_transfers, this, _1), "Show incoming transfers");
  m_consoleHandler.setHandler("comments", boost::bind(&simple_wallet::listComments, this, _1), "Show all transaction's comments");
  m_consoleHandler.setHandler("list", boost::bind(&simple_wallet::listTransfers, this, _1), "Show all known transfers");
  m_consoleHandler.setHandler("payments", boost::bind(&simple_wallet::show_payments, this, _1), "payments <payment_id_1> [<payment_id_2> ... <payment_id_N>] - Show payments <payment_id_1>, ... <payment_id_N>");
  m_consoleHandler.setHandler("top", boost::bind(&simple_wallet::show_blockchain_height, this, _1), "Show blockchain height");
  m_consoleHandler.setHandler("bc_height", boost::bind(&simple_wallet::show_blockchain_height, this, _1), "Show blockchain height");
  m_consoleHandler.setHandler("transfer", boost::bind(&simple_wallet::transfer, this, _1),
    "transfer <mixin_count> <addr_1> <amount_1> [<addr_2> <amount_2> ... <addr_N> <amount_N>] [-p payment_id] [-f fee]"
    " - Transfer <amount_1>,... <amount_N> to <address_1>,... <address_N>, respectively. "
    "<mixin_count> is the number of transactions yours is indistinguishable from (from 0 to maximum available)");
  m_consoleHandler.setHandler("log", boost::bind(&simple_wallet::set_log, this, _1), "log <level> - Change current log level, <level> is a number 0-4");
  m_consoleHandler.setHandler("address", boost::bind(&simple_wallet::print_address, this, _1), "Show current wallet's public address");
  m_consoleHandler.setHandler("paper", boost::bind(&simple_wallet::paper, this, _1), "Show a new paper wallet data");
  m_consoleHandler.setHandler("paper_prn", boost::bind(&simple_wallet::paper_prn, this, _1), "Save a new paper wallet data to a file");
  m_consoleHandler.setHandler("keys", boost::bind(&simple_wallet::print_keys, this, _1), "Show current wallet's keys");
  m_consoleHandler.setHandler("seed", boost::bind(&simple_wallet::print_seed, this, _1), "Show current wallet's recovery seed");
  m_consoleHandler.setHandler("save", boost::bind(&simple_wallet::save, this, _1), "Save wallet synchronized data");
  m_consoleHandler.setHandler("reset", boost::bind(&simple_wallet::reset, this, _1), "Discard cache data and start synchronizing from the start");
  m_consoleHandler.setHandler("help", boost::bind(&simple_wallet::help, this, _1), "Show this help");
  m_consoleHandler.setHandler("exit", boost::bind(&simple_wallet::exit, this, _1), "Close wallet");
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::set_log(const std::vector<std::string> &args) {
  if (args.size() != 1) {
    fail_msg_writer() << "use: log <log_level_number_0-4>";
    return true;
  }

  uint16_t l = 0;
  if (!Common::fromString(args[0], l)) {
    fail_msg_writer() << "wrong number format, use: log <log_level_number_0-4>";
    return true;
  }
 
  if (l > Logging::TRACE) {
    fail_msg_writer() << "wrong number range, use: log <log_level_number_0-4>";
    return true;
  }

  logManager.setMaxLevel(static_cast<Logging::Level>(l));
  return true;
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::init(const boost::program_options::variables_map& vm) 
{
  handle_command_line(vm);
	m_is_copy = 0;
  if (!m_daemon_address.empty() && (!m_daemon_host.empty() || 0 != m_daemon_port)) 
  {
    fail_msg_writer() << "you can't specify daemon host or port several times";
    return false;
  }

  m_is_recovery_mode = 0;
  
  if (m_generate_new.empty() && m_wallet_file_arg.empty()) 
  {
    char c;
	
	if (m_restore_seed) {c = 'S';}
	if (m_restore_keys) {c = 'K';}
	if (m_restore_legacy) {c = 'L';}
	
	if (!m_restore_seed && !m_restore_keys && !m_restore_legacy)
	{		
		std::cout 
			<< "No arguments was specified." << std::endl 
			<< "What do you want to do?" << std::endl 
			<< std::endl 
			<< "[O]pen existing wallet" << std::endl 
			<< "[G]enerate new wallet file" << std::endl 
			<< "Recover wallet using [S]eed" << std::endl 
			<< "Recover wallet using hexadecimal wallet [K]eys" << std::endl 
			<< "Recover wallet using [L]egacy recovery key" << std::endl 
			<< "[E]xit" << std::endl << std::endl;
			
		do 
			{
				std::string answer;
				std::getline(std::cin, answer);
				c = answer[0];

				if (!(c == 'O' || c == 'G' || c == 'S' || c == 'K' || c == 'L' || c == 'E' || c == 'o' || c == 'g' || c == 's' || c == 'k' || c == 'l' || c == 'e')) 
					{
						std::cout << "Unknown command: " << c << std::endl;
					} 
				else 
					{
						break;
					}
			} 
		while (true);		
	}

    if (c == 'E' || c == 'e') 
		{			
			return false;
		}

    std::cout << "Specify wallet file name (e.g., sld-first.wallet).\n";
    std::string userInput;
    do {
      std::cout << "Wallet file name: ";
      std::getline(std::cin, userInput);
      boost::algorithm::trim(userInput);
    } while (userInput.empty());

    if (c == 'g' || c == 's' || c == 'k' || c == 'l' || c == 'G' || c == 'S' || c == 'K' || c == 'L') 
		{
		  m_generate_new = userInput;
		}
	else 
		{
		  m_wallet_file_arg = userInput;
		}
		
	if (c == 'S' || c == 's') {
		m_restore_seed = 1;
		m_restore_keys = 0;
		m_restore_legacy = 0;
	}		
	
	if (c == 'K' || c == 'k') {
		m_restore_seed = 0;
		m_restore_keys = 1;
		m_restore_legacy = 0;
	}		
	
	if (c == 'L' || c == 'l') {
		m_restore_seed = 0;
		m_restore_keys = 0;
		m_restore_legacy = 1;
	}			
  }
	if (m_generate_new.empty() && m_wallet_file_arg.empty()) 
	{
	}

  if (!m_generate_new.empty() && !m_wallet_file_arg.empty()) {
    fail_msg_writer() << "you can't specify 'generate-new-wallet' or 'restore-* ...' and 'wallet-file' arguments simultaneously";
    return false;
  }

  std::string walletFileName;
  if (!m_generate_new.empty()) {
    std::string ignoredString;
    WalletHelper::prepareFileNames(m_generate_new, ignoredString, walletFileName);
    boost::system::error_code ignore;
    if (boost::filesystem::exists(walletFileName, ignore)) {
      fail_msg_writer() << walletFileName << " already exists";
      return false;
    }
  }

  if (m_daemon_host.empty())
    m_daemon_host = "localhost";
  if (!m_daemon_port)
    m_daemon_port = RPC_DEFAULT_PORT;
  
  if (!m_daemon_address.empty()) {
    if (!parseUrlAddress(m_daemon_address, m_daemon_host, m_daemon_port)) {
      fail_msg_writer() << "failed to parse daemon address: " << m_daemon_address;
      return false;
    }
  } else {
    m_daemon_address = std::string("http://") + m_daemon_host + ":" + std::to_string(m_daemon_port);
  }

  Tools::PasswordContainer pwd_container;
  if (command_line::has_arg(vm, arg_password)) {
    pwd_container.password(command_line::get_arg(vm, arg_password));
  } else if (!pwd_container.read_password()) {
    fail_msg_writer() << "failed to read wallet password";
    return false;
  }

  this->m_node.reset(new NodeRpcProxy(m_daemon_host, m_daemon_port));

  std::promise<std::error_code> errorPromise;
  std::future<std::error_code> f_error = errorPromise.get_future();
  auto callback = [&errorPromise](std::error_code e) {errorPromise.set_value(e); };

  m_node->addObserver(static_cast<INodeRpcProxyObserver*>(this));
  m_node->init(callback);
  auto error = f_error.get();
  if (error) {
    fail_msg_writer() << "failed to init NodeRPCProxy: " << error.message();
    return false;
  }

  if (!m_generate_new.empty() || m_restore_seed || m_restore_keys || m_restore_legacy ) // generate or resrore mode ----------------
  {
    std::string walletBackupFile = prepareWalletBackupFilename(m_generate_new);
    boost::system::error_code ignore;
	
    if (boost::filesystem::exists(walletBackupFile, ignore)) {
      logger(ERROR, BRIGHT_RED) << "Backup file already exists: " + walletBackupFile;
      return false;
    }
// prep to restore here ____________________________________________________________________________________________________________
    if (m_restore_seed) //seed restore
    {
		m_is_recovery_mode = 1;
		if (m_classic) //check 2-rand .........................................
		{
			std::cout 
				<< maroon << "Cannot restore seed in CLASSIC wallet mode." << std::endl 
				<< khaki << "--classic parameter will be ignored." << std::endl << grey;
			
			m_classic = 0;
		}

		if (m_seed.empty()) // chk empty seed and enter it ......................
		{
			std::cout << teal << "Enter mnemonic seed:" << std::endl << magenta;
			std::getline(std::cin, m_seed);
			std::cout << grey;
			boost::algorithm::trim(m_seed);
			
			if (m_seed.empty())
			{
				fail_msg_writer() << "set a recovery seed parameter with the --seed=\"words list here\"";
				return false;
			}
		}
//		std::cout << blue << "Seed: " << m_seed << std::endl;
		
		if (!Crypto::ElectrumWords::words_to_bytes(m_seed, m_spend_secret_key)) // chk seed :::::::::
		{
			fail_msg_writer() << "seed word list failed verification";
			return false;
		}
    }
	
    if (m_restore_keys) //keys restore
    {
		m_is_recovery_mode = 1;
		m_classic = 0; //always restore secondary key
		m_is_copy = 1;
		if (m_spend_key.empty()) // chk empty Private Spend key and enter it ......................
		{
			std::cout << teal << "Enter Private Spend Key:" << std::endl << magenta;
			std::getline(std::cin, m_spend_key);
			std::cout << grey;
			boost::algorithm::trim(m_spend_key);
			
			if (m_spend_key.empty())
			{
				fail_msg_writer() << "set a Private Spend key parameter with the --spend-key=\"hexadecimal_key\"";
				return false;
			}
		}
		
		if (m_view_key.empty()) // chk empty Private View key and enter it ......................
		{
			std::cout << teal << "Enter Private View Key:" << std::endl << magenta;
			std::getline(std::cin, m_view_key);
			std::cout << grey;
			boost::algorithm::trim(m_view_key);
			
			if (m_view_key.empty())
			{
				fail_msg_writer() << "set a Private View key parameter with the --viev-key=hexadecimal_key";
				return false;
			}
		}
				
		if (!Common::podFromHex(m_spend_key,m_spend_secret_key.data))
		{
			fail_msg_writer() << "Private Spend Key failed";
			return false;
		}
		
		if (!Common::podFromHex(m_view_key,m_view_secret_key.data))
		{
			fail_msg_writer() << "Private View Key failed";
			return false;
		}
		
		if (!Crypto::check_skey(m_spend_secret_key)) // chk Private Spend key :::::::::
		{
			fail_msg_writer() << "Private Spend Key is invalid";
			return false;
		}
		
		if (!Crypto::check_skey(m_view_secret_key)) // chk View Spend key :::::::::
		{
			fail_msg_writer() << "Private View Key is invalid";
			return false;
		}		
    }
   
if (m_restore_legacy) //legacy restore
    {
		m_is_recovery_mode = 1;
		m_classic = 0; //always restore secondary key
		m_is_copy = 1;
		if (m_legacy_key.empty()) // chk empty seed and enter it ......................
		{
			std::cout << teal << "Enter Legacy Recovery Key:" << std::endl << magenta;
			std::getline(std::cin, m_legacy_key);
			std::cout << grey;
			boost::algorithm::trim(m_legacy_key);
			
			if (m_legacy_key.empty())
			{
				fail_msg_writer() << "set a Legacy Recovery Key parameter with the --legacy-key=legacy_key";
				return false;
			}
		}
		
		if (!m_legacy_key.empty())
		{

			uint64_t addressPrefix;
			std::string data;
			CryptoNote::AccountKeys keys;
			
			addressPrefix = parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX;

			if (!Tools::Base58::decode_addr(m_legacy_key, addressPrefix, data))
			{
				fail_msg_writer() << "can't decode key from Base58";
				return false;
			}
			
			if (!data.size() == sizeof(keys))
			{
				fail_msg_writer() << "data.size() == sizeof(keys)";
				return false;
			}
			
			std::memcpy(&keys, data.data(), sizeof(keys));
			m_spend_secret_key = keys.spendSecretKey;
			m_view_secret_key = keys.viewSecretKey;
			
		}
    }
// prep finished here ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    if (!new_wallet(
		walletFileName, 
		pwd_container.password(),
		m_spend_secret_key,
		m_view_secret_key,
		m_is_recovery_mode,
		m_is_copy,
		!m_classic)) 
	{
		logger(ERROR, BRIGHT_RED) << "account creation failed";
		return false;
    }
//todo - last action while gen or rest .............................................................................................
    AccountKeys keys;
    m_wallet->getAccountKeys(keys);
		
	std::string electrum_words;
	bool success = try_seed(electrum_words,keys);
		
    if (!writeBackupFile(
							walletBackupFile, 
							m_wallet->getAddress(),
							walletFileName,
							pwd_container.password(),
							keys,
							success,
							electrum_words
						)
		)
						
	{
      logger(WARNING, BRIGHT_RED) << "Couldn't write wallet backup file: " + walletBackupFile;
    }
  } 
	else // open file mode -----------------------------------------------------------------------------------------------------------------------
	{
		m_wallet.reset(new WalletLegacy(m_currency, *m_node));

		try 
		{
			m_wallet_file = tryToOpenWalletOrLoadKeysOrThrow(logger, m_wallet, m_wallet_file_arg, pwd_container.password());
		} 
		catch (const std::exception& e) 
		{
			fail_msg_writer() << "failed to load wallet: " << e.what();
			return false;
		}

		m_wallet->addObserver(this);
		m_node->addObserver(static_cast<INodeObserver*>(this));

		logger(INFO, BRIGHT_WHITE) << "Opened wallet: " << m_wallet->getAddress();

		std::cout 
		<< "======================================================================\n"
		<< "Use \"help\" command to see the list of available commands.\n"
		<< "======================================================================" << std::endl << grey;
	}

  return true;
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::deinit() {
  m_wallet->removeObserver(this);
  m_node->removeObserver(static_cast<INodeObserver*>(this));
  m_node->removeObserver(static_cast<INodeRpcProxyObserver*>(this));

  if (!m_wallet.get())
    return true;

  return close_wallet();
}
//----------------------------------------------------------------------------------------------------
void simple_wallet::handle_command_line(const boost::program_options::variables_map& vm) 
{
  m_wallet_file_arg = command_line::get_arg(vm, arg_wallet_file);
  m_generate_new = command_line::get_arg(vm, arg_generate_new_wallet);
  m_daemon_address = command_line::get_arg(vm, arg_daemon_address);
  m_daemon_host = command_line::get_arg(vm, arg_daemon_host);
  m_daemon_port = command_line::get_arg(vm, arg_daemon_port);
//$$$$
  m_restore_seed = command_line::get_arg(vm, arg_restore_seed);
  m_restore_keys = command_line::get_arg(vm, arg_restore_keys);
  m_restore_legacy = command_line::get_arg(vm, arg_restore_legacy);
  m_classic = command_line::get_arg(vm, arg_classic);
  m_seed = command_line::get_arg(vm, arg_seed);
  m_legacy_key = command_line::get_arg(vm, arg_legacy_key);
  m_spend_key = command_line::get_arg(vm, arg_spend_key);
  m_view_key = command_line::get_arg(vm, arg_view_key);
//$$$$
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::new_wallet
	(
		const std::string &wallet_file, 
		const std::string& password, 
		const Crypto::SecretKey& recovery_key,
		const Crypto::SecretKey& secondary_key,
		bool is_recovery, 
		bool is_copy,
		bool is_deterministic
	) 
{
  m_wallet_file = wallet_file;

  m_wallet.reset(new WalletLegacy(m_currency, *m_node.get()));
  m_node->addObserver(static_cast<INodeObserver*>(this));
  m_wallet->addObserver(this);
  try {
    m_initResultPromise.reset(new std::promise<std::error_code>());
    std::future<std::error_code> f_initError = m_initResultPromise->get_future();
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//    m_wallet->initAndGenerate(password);
// m_classic - if true - generating old-style wallet with 2 random private keys
//		Crypto::SecretKey rkey;
		m_wallet->initAndGenerateOrRecover(password,recovery_key,secondary_key,is_recovery,is_copy,is_deterministic);
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    auto initError = f_initError.get();
    m_initResultPromise.reset(nullptr);
    if (initError) {
      fail_msg_writer() << "failed to generate new wallet: " << initError.message();
      return false;
    }

    try {
      CryptoNote::WalletHelper::storeWallet(*m_wallet, m_wallet_file);
    } catch (std::exception& e) {
      fail_msg_writer() << "failed to save new wallet: " << e.what();
      throw;
    }

    AccountKeys keys;
    m_wallet->getAccountKeys(keys);

    logger(INFO, BRIGHT_YELLOW) << "New wallet prepared!" << std::endl;
	logger(INFO, CYAN)  << "Wallet address is: " << m_wallet->getAddress() << std::endl;
	
	std::string electrum_words;
	bool success = try_seed(electrum_words,keys);
	if (success) 
		{
			std::cout << std::endl << lime << "The wallet is MODERN." << std::endl << grey;		
		}
	else
		{
			std::cout << std::endl << red << "The wallet is CLASSIC." << std::endl << grey;
		}	
		
  }
  catch (const std::exception& e) {
    fail_msg_writer() << "failed to generate new wallet: " << e.what();
    return false;
  }

  std::cout << std::endl 
    << "======================================================================\n" 
    << "Your new wallet has been generated.\n" 
    << "Use \"help\" command to see the list of available commands.\n" 
    << "Always use \"exit\" command when closing wallet app to save\n" 
    << "current session's state. \n" 
    << "Otherwise, you will possibly need to synchronize your wallet again.\n" 
    << "Your wallet key is NOT under risk anyway.\n" 
    << "======================================================================" << std::endl << grey;
  return true;
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::close_wallet()
{
  try {
    CryptoNote::WalletHelper::storeWallet(*m_wallet, m_wallet_file);
  } catch (const std::exception& e) {
    fail_msg_writer() << e.what();
    return false;
  }

  m_wallet->removeObserver(this);
  m_wallet->shutdown();

  return true;
}

//----------------------------------------------------------------------------------------------------
bool simple_wallet::save(const std::vector<std::string> &args)
{
  try {
    CryptoNote::WalletHelper::storeWallet(*m_wallet, m_wallet_file);
    success_msg_writer() << "Wallet data saved";
  } catch (const std::exception& e) {
    fail_msg_writer() << e.what();
  }

  return true;
}

bool simple_wallet::reset(const std::vector<std::string> &args) {
  {
    std::unique_lock<std::mutex> lock(m_walletSynchronizedMutex);
    m_walletSynchronized = false;
  }

  m_wallet->reset();
  success_msg_writer(true) << "Reset completed successfully.";

  std::unique_lock<std::mutex> lock(m_walletSynchronizedMutex);
  while (!m_walletSynchronized) {
    m_walletSynchronizedCV.wait(lock);
  }

  std::cout << std::endl;

  return true;
}

bool simple_wallet::start_mining(const std::vector<std::string>& args) {
  COMMAND_RPC_START_MINING::request req;
  req.miner_address = m_wallet->getAddress();

  bool ok = true;
  size_t max_mining_threads_count = (std::max)(std::thread::hardware_concurrency(), static_cast<unsigned>(2));
  if (0 == args.size()) {
    req.threads_count = 1;
  } else if (1 == args.size()) {
    uint16_t num = 1;
    ok = Common::fromString(args[0], num);
    ok = ok && (1 <= num && num <= max_mining_threads_count);
    req.threads_count = num;
  } else {
    ok = false;
  }

  if (!ok) {
    fail_msg_writer() << "invalid arguments. Please use start_mining [<number_of_threads>], " <<
      "<number_of_threads> should be from 1 to " << max_mining_threads_count;
    return true;
  }


  COMMAND_RPC_START_MINING::response res;

  try {
    HttpClient httpClient(m_dispatcher, m_daemon_host, m_daemon_port);

    invokeJsonCommand(httpClient, "/start_mining", req, res);

    std::string err = interpret_rpc_response(true, res.status);
    if (err.empty())
      success_msg_writer() << "Mining started in daemon";
    else
      fail_msg_writer() << "mining has NOT been started: " << err;

  } catch (const ConnectException&) {
    printConnectionError();
  } catch (const std::exception& e) {
    fail_msg_writer() << "Failed to invoke rpc method: " << e.what();
  }

  return true;
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::stop_mining(const std::vector<std::string>& args)
{
  COMMAND_RPC_STOP_MINING::request req;
  COMMAND_RPC_STOP_MINING::response res;

  try {
    HttpClient httpClient(m_dispatcher, m_daemon_host, m_daemon_port);

    invokeJsonCommand(httpClient, "/stop_mining", req, res);
    std::string err = interpret_rpc_response(true, res.status);
    if (err.empty())
      success_msg_writer() << "Mining stopped in daemon";
    else
      fail_msg_writer() << "mining has NOT been stopped: " << err;
  } catch (const ConnectException&) {
    printConnectionError();
  } catch (const std::exception& e) {
    fail_msg_writer() << "Failed to invoke rpc method: " << e.what();
  }

  return true;
}
//----------------------------------------------------------------------------------------------------
void simple_wallet::initCompleted(std::error_code result) {
  if (m_initResultPromise.get() != nullptr) {
    m_initResultPromise->set_value(result);
  }
}
//----------------------------------------------------------------------------------------------------
void simple_wallet::connectionStatusUpdated(bool connected) {
  if (connected) {
    logger(INFO, GREEN) << "Wallet connected to daemon.";
  } else {
    printConnectionError();
  }
}
//----------------------------------------------------------------------------------------------------
void simple_wallet::externalTransactionCreated(CryptoNote::TransactionId transactionId)  
{
	WalletLegacyTransaction txInfo;
	m_wallet->getTransaction(transactionId, txInfo);
	std::stringstream logPrefix;

	if (txInfo.blockHeight == WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT) 
		{
			logPrefix << "Unconfirmed";
		} 
	else 
		{
			logPrefix << get_tx_time_str(txInfo) << " @ " << txInfo.blockHeight;
		}

	if (txInfo.totalAmount >= 0) 
		{
			std::cout << green <<
			logPrefix.str() << " = " << Common::podToHex(txInfo.hash) <<
			" +" << m_currency.formatAmount(txInfo.totalAmount) << std::endl << grey;
		} 
	else 
		{
			std::cout << maroon <<
			logPrefix.str() << " = " << Common::podToHex(txInfo.hash) <<
			" -" << m_currency.formatAmount(static_cast<uint64_t>(-txInfo.totalAmount)) << std::endl << grey;
		}
//$$$$
	if (txInfo.messages.size() > 0) 
	{
		std::cout  << teal << " COMMENTS >>" << std::endl;
		for(int i = 0; i < txInfo.messages.size(); i++)
			std::cout << cyan << "  " << txInfo.messages[i] << std::endl << grey;
	}
//$$$$

  if (txInfo.blockHeight == WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT) {
    m_refresh_progress_reporter.update(m_node->getLastLocalBlockHeight(), true);
  } else {
    m_refresh_progress_reporter.update(txInfo.blockHeight, true);
  }
}
//----------------------------------------------------------------------------------------------------
void simple_wallet::synchronizationCompleted(std::error_code result) {
  std::unique_lock<std::mutex> lock(m_walletSynchronizedMutex);
  m_walletSynchronized = true;
  m_walletSynchronizedCV.notify_one();
}

void simple_wallet::synchronizationProgressUpdated(uint32_t current, uint32_t total) {
  std::unique_lock<std::mutex> lock(m_walletSynchronizedMutex);
  if (!m_walletSynchronized) {
    m_refresh_progress_reporter.update(current, false);
  }
}

bool simple_wallet::show_balance(const std::vector<std::string>& args/* = std::vector<std::string>()*/) {
  success_msg_writer() << "Available balance: " << m_currency.formatAmount(m_wallet->actualBalance()) <<
    ", locked amount: " << m_currency.formatAmount(m_wallet->pendingBalance());

  return true;
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::show_incoming_transfers(const std::vector<std::string>& args) 
{
	bool hasTransfers = false;
	std::cout << std::endl << yellow << "INCOMING TRANSFERS:" << std::endl << std::endl << grey; 
	
	size_t transactionsCount = m_wallet->getTransactionCount();
  for (size_t trantransactionNumber = 0; trantransactionNumber < transactionsCount; ++trantransactionNumber) 
  
	{
	WalletLegacyTransaction txInfo;
	m_wallet->getTransaction(trantransactionNumber, txInfo);
	std::string suxx;
//	suxx = get_tx_time_str(txInfo);
	
	if (txInfo.totalAmount < 0) continue;
	hasTransfers = true;

	std::cout << green 
		<< Common::podToHex(txInfo.hash) << " @ " 
		<< get_tx_time_str(txInfo) << " = " 
		<< std::setw(17) << m_currency.formatAmount(txInfo.totalAmount) << std::endl << grey;
	}

  if (!hasTransfers) success_msg_writer() << "No incoming transfers yet :(";
  return true;
}
/////////////////////////////////////////////////////
bool simple_wallet::listComments(const std::vector<std::string>& args) 
{
	size_t transactionsCount = m_wallet->getTransactionCount();
	std::cout << std::endl << yellow <<  "COMMENTS:" << std::endl;
	
	for (size_t trantransactionNumber = 0; trantransactionNumber < transactionsCount; ++trantransactionNumber) 
		{
			WalletLegacyTransaction txInfo;
			m_wallet->getTransaction(trantransactionNumber, txInfo);

			if (txInfo.state != WalletLegacyTransactionState::Active || txInfo.blockHeight == WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT) 
			{
			  continue;
			}
			printTxComments(txInfo);
		}
	return true;
}
/////////////////////////////////////////////////////
bool simple_wallet::listTransfers(const std::vector<std::string>& args) {
  bool haveTransfers = false;

  size_t transactionsCount = m_wallet->getTransactionCount();
	std::cout << std::endl << yellow << "TRANSFERS:" << std::endl << std::endl << grey;
	
  for (size_t trantransactionNumber = 0; trantransactionNumber < transactionsCount; ++trantransactionNumber) {
    WalletLegacyTransaction txInfo;
    m_wallet->getTransaction(trantransactionNumber, txInfo);
    if (txInfo.state != WalletLegacyTransactionState::Active || txInfo.blockHeight == WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT) {
      continue;
    }

    if (!haveTransfers) {
      printListTransfersHeader(logger);
      haveTransfers = true;
    }

    printListTransfersItem(logger, txInfo, *m_wallet, m_currency);
  }

  if (!haveTransfers) {
    success_msg_writer() << "No transfers";
  }

  return true;
}
//////////////////////////////////////////////////////////////////
bool simple_wallet::show_payments(const std::vector<std::string> &args) {
  if (args.empty()) {
    fail_msg_writer() << "expected at least one payment ID";
    return true;
  }

  try {
	std::cout << std::endl << yellow <<  "PAYMENTS:" << std::endl << std::endl << grey; 
	  
    auto hashes = args;
    std::sort(std::begin(hashes), std::end(hashes));
    hashes.erase(std::unique(std::begin(hashes), std::end(hashes)), std::end(hashes));
    std::vector<PaymentId> paymentIds;
    paymentIds.reserve(hashes.size());
    std::transform(std::begin(hashes), std::end(hashes), std::back_inserter(paymentIds), [](const std::string& arg) {
      PaymentId paymentId;
      if (!CryptoNote::parsePaymentId(arg, paymentId)) {
        throw std::runtime_error("payment ID has invalid format: \"" + arg + "\", expected 64-character string");
      }

      return paymentId;
    });

    std::cout << "                            payment                             \t" <<
      "                          transaction                           \t" <<
      "  height\t       amount        ";

    auto payments = m_wallet->getTransactionsByPaymentIds(paymentIds);

    for (auto& payment : payments) {
      for (auto& transaction : payment.transactions) {
        success_msg_writer(true) << 
          Common::podToHex(payment.paymentId) << '\t' <<
          Common::podToHex(transaction.hash) << '\t' <<
          std::setw(8) << transaction.blockHeight << '\t' << 
          std::setw(21) << m_currency.formatAmount(transaction.totalAmount);
      }

      if (payment.transactions.empty()) {
        success_msg_writer() << "No payments with id " << Common::podToHex(payment.paymentId);
      }
    }
  } catch (std::exception& e) {
    fail_msg_writer() << "show_payments exception: " << e.what();
  }

  return true;
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::show_blockchain_height(const std::vector<std::string>& args) {
  try {
    uint64_t bc_height = m_node->getLastLocalBlockHeight();
    success_msg_writer() << bc_height;
  } catch (std::exception &e) {
    fail_msg_writer() << "failed to get blockchain height: " << e.what();
  }

  return true;
}
//----------------------------------------------------------------------------------------------------
std::string simple_wallet::resolveAlias(const std::string& aliasUrl) {
  std::string host;
  std::string uri;

  if (!splitUrlToHostAndUri(aliasUrl, host, uri)) {
    throw std::runtime_error("Invalid url");
  }

  HttpClient httpClient(m_dispatcher, host, 80);

  HttpRequest req;
  HttpResponse res;

  req.setUrl(uri);
  httpClient.request(req, res);

  if (res.getStatus() != HttpResponse::STATUS_200) {
    throw std::runtime_error("Remote server returned code " + std::to_string(res.getStatus()));
  }

  std::string address;
  if (!processServerAliasResponse(res.getBody(), address)) {
    throw std::runtime_error("Failed to parse server response");
  }

  return address;
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::transfer(const std::vector<std::string> &args) {
//std::cout << blue << "simple_wallet::transfer" << std::endl;//z
  try {
    TransferCommand cmd(m_currency);

    if (!cmd.parseArguments(logger, args))
      return true;

    for (auto& kv: cmd.aliases) {
      std::string address;

      try {
        address = resolveAlias(kv.first);

        AccountPublicAddress ignore;
        if (!m_currency.parseAccountAddressString(address, ignore)) {
          throw std::runtime_error("Address \"" + address + "\" is invalid");
        }
      } catch (std::exception& e) {
        fail_msg_writer() << "Couldn't resolve alias: " << e.what() << ", alias: " << kv.first;
        return true;
      }

      for (auto& transfer: kv.second) {
        transfer.address = address;
      }
    }

    if (!cmd.aliases.empty()) {
      if (!askAliasesTransfersConfirmation(cmd.aliases, m_currency)) {
        return true;
      }

      for (auto& kv: cmd.aliases) {
        std::copy(std::move_iterator<std::vector<WalletLegacyTransfer>::iterator>(kv.second.begin()),
                  std::move_iterator<std::vector<WalletLegacyTransfer>::iterator>(kv.second.end()),
                  std::back_inserter(cmd.dsts));
      }
    }

    std::vector<TransactionMessage> messages;
    for (auto dst : cmd.dsts) {
      for (auto msg : cmd.messages) {
        messages.emplace_back(TransactionMessage{ msg, dst.address });
      }
    }

    uint64_t ttl = 0;
    if (cmd.ttl != 0) {
      ttl = static_cast<uint64_t>(time(nullptr)) + cmd.ttl;
    }

    CryptoNote::WalletHelper::SendCompleteResultObserver sent;

    std::string extraString;
    std::copy(cmd.extra.begin(), cmd.extra.end(), std::back_inserter(extraString));

    WalletHelper::IWalletRemoveObserverGuard removeGuard(*m_wallet, sent);
//std::cout << blue << "simple_wallet::transfer = " << cmd.fee << std::endl;//z
    CryptoNote::TransactionId tx = m_wallet->sendTransaction(cmd.dsts, cmd.fee, extraString, cmd.fake_outs_count, 0, messages, ttl);
    if (tx == WALLET_LEGACY_INVALID_TRANSACTION_ID) {
      fail_msg_writer() << "Can't send money";
      return true;
    }

    std::error_code sendError = sent.wait(tx);
    removeGuard.removeObserver();

    if (sendError) {
      fail_msg_writer() << sendError.message();
      return true;
    }

    CryptoNote::WalletLegacyTransaction txInfo;
    m_wallet->getTransaction(tx, txInfo);
    success_msg_writer(true) << "Money successfully sent, transaction " << Common::podToHex(txInfo.hash);

    try {
      CryptoNote::WalletHelper::storeWallet(*m_wallet, m_wallet_file);
    } catch (const std::exception& e) {
      fail_msg_writer() << e.what();
      return true;
    }
  } catch (const std::system_error& e) {
    fail_msg_writer() << e.what();
  } catch (const std::exception& e) {
    fail_msg_writer() << e.what();
  } catch (...) {
    fail_msg_writer() << "unknown error";
  }

  return true;
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::run() {
  {
    std::unique_lock<std::mutex> lock(m_walletSynchronizedMutex);
    while (!m_walletSynchronized) {
      m_walletSynchronizedCV.wait(lock);
    }
  }

  std::cout << std::endl;

  std::string addr_start = m_wallet->getAddress().substr(0, 9);
  m_consoleHandler.start(false, addr_start + " >> ", Common::Console::Color::Yellow);
  return true;
}
//----------------------------------------------------------------------------------------------------
void simple_wallet::stop() {
  m_consoleHandler.requestStop();
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::print_keys(const std::vector<std::string> &args) 
{
    AccountKeys keys;
    m_wallet->getAccountKeys(keys);

	std::cout << std::endl << magenta 
		<< "!!! AHTUNG !!!" << std::endl << std::endl << teal 
		<< "The following 2 upper keys can be used to recover access to your wallet." << std::endl 
		<< "Please write them down and store them somewhere safe and secure." << std::endl 
		<< "Please do not store them in your email or on file storage services outside of your immediate control." << std::endl;

	print_keys_to_stream(keys, "Keys are: ", 0, std::cout);
	
	return true;
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::print_seed(const std::vector<std::string> &args) 
{
	std::string electrum_words;
    AccountKeys keys;
    m_wallet->getAccountKeys(keys);

	bool success = try_seed(electrum_words,keys);

	if (success) 
	{
		std::cout << std::endl << magenta 
			<< "!!! AHTUNG !!!" << std::endl << std::endl << teal 
			<< "The following 24 words can be used to recover access to your wallet." << std::endl 
			<< "Please write them down and store them somewhere safe and secure." << std::endl 
			<< "Please do not store them in your email or on file storage services outside of your immediate control." << std::endl;
			
		std::cout << std::endl << maroon << electrum_words << std::endl << std::endl;      
	}
	else
	{
		std::cout 
			<< std::endl << red << "The wallet is CLASSIC." << std::endl 
			<< teal << "Cannot display seed." << std::endl 
			<< "Use " << khaki << "keys" << teal << " command instead." << std::endl << std::endl;
	}

	return true;
}
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
bool simple_wallet::try_seed(std::string& electrum_words, AccountKeys keys)
{
	Crypto::ElectrumWords::bytes_to_words(keys.spendSecretKey, electrum_words);	
	Crypto::SecretKey second;
	
	keccak((uint8_t *)&keys.spendSecretKey, sizeof(Crypto::SecretKey), (uint8_t *)&second, sizeof(Crypto::SecretKey));
	
	sc_reduce32((uint8_t *)&second);
	
	return memcmp(second.data,keys.viewSecretKey.data, sizeof(Crypto::SecretKey)) == 0;	
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::print_address(const std::vector<std::string> &args) 
{
	std::cout << std::endl << yellow << "ADDRESS:" << std::endl << std::endl << grey; 	
	std::cout << m_wallet->getAddress()<< std::endl << std::endl << grey; 
	return true;
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::paper(const std::vector<std::string> &args) 
{
	return print_paper_wallet("");
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::paper_prn(const std::vector<std::string> &args) 
{
	std::string fn = z_get_now_time_str("%Y-%m-%d-%H-%M-%S")+"-paper-wallet.txt";
	std::cout << std::endl << teal << "Paper wallet stored in: " << cyan << fn << teal << " file." << std::endl << grey; 	
	return print_paper_wallet(fn);
}
//----------------------------------------------------------------------------------------------------
bool simple_wallet::process_command(const std::vector<std::string> &args) {
  return m_consoleHandler.runCommand(args);
}

void simple_wallet::printConnectionError() const {
  fail_msg_writer() << "wallet failed to connect to daemon (" << m_daemon_address << ").";
}


int main(int argc, char* argv[]) {
#ifdef WIN32
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  po::options_description desc_general("General options");
  command_line::add_arg(desc_general, command_line::arg_help);
  command_line::add_arg(desc_general, command_line::arg_version);

  po::options_description desc_params("Wallet options");
  command_line::add_arg(desc_params, arg_wallet_file);
  command_line::add_arg(desc_params, arg_generate_new_wallet);
  command_line::add_arg(desc_params, arg_password);
  command_line::add_arg(desc_params, arg_daemon_address);
  command_line::add_arg(desc_params, arg_daemon_host);
  command_line::add_arg(desc_params, arg_daemon_port);
  command_line::add_arg(desc_params, arg_command);
  command_line::add_arg(desc_params, arg_log_level);
  command_line::add_arg(desc_params, arg_testnet);
//$$$$
  command_line::add_arg(desc_params, arg_classic);
  command_line::add_arg(desc_params, arg_restore_seed);
  command_line::add_arg(desc_params, arg_restore_keys);
  command_line::add_arg(desc_params, arg_restore_legacy);
  command_line::add_arg(desc_params, arg_seed);
  command_line::add_arg(desc_params, arg_spend_key);
  command_line::add_arg(desc_params, arg_view_key);
  command_line::add_arg(desc_params, arg_legacy_key);
//$$$$
  
  Tools::wallet_rpc_server::init_options(desc_params);

  po::positional_options_description positional_options;
  positional_options.add(arg_command.name, -1);

  po::options_description desc_all;
  desc_all.add(desc_general).add(desc_params);

  Logging::LoggerManager logManager;
  Logging::LoggerRef logger(logManager, "simplewallet");
  System::Dispatcher dispatcher;

  po::variables_map vm;

  bool r = command_line::handle_error_helper(desc_all, [&]() {
    po::store(command_line::parse_command_line(argc, argv, desc_general, true), vm);

    if (command_line::get_arg(vm, command_line::arg_help)) 
		{
			CryptoNote::Currency tmp_currency = CryptoNote::CurrencyBuilder(logManager).currency();
			CryptoNote::simple_wallet tmp_wallet(dispatcher, tmp_currency, logManager);
			std::cout << yellow << CRYPTONOTE_NAME << " wallet " << teal << PROJECT_VERSION_LONG << grey << std::endl << std::endl;
			std::cout << "Usage: "<< CRYPTONOTE_TICKER <<" [w --wallet-file=<file>|--generate-new-wallet=<file>] [--daemon-address=<host>:<port>] [<COMMAND>]";
			std::cout << desc_all << '\n' << tmp_wallet.get_commands_str();
			return false;
		} 
	else if (command_line::get_arg(vm, command_line::arg_version))  
		{
			std::cout << cyan << CRYPTONOTE_NAME << " wallet " << teal << PROJECT_VERSION_LONG << grey << std::endl << std::endl;
			return false;
		}

    auto parser = po::command_line_parser(argc, argv).options(desc_params).positional(positional_options);
    po::store(parser.run(), vm);
    po::notify(vm);
    return true;
  });

  if (!r)
    return 1;

  //set up logging options
  Level logLevel = DEBUGGING;

  if (command_line::has_arg(vm, arg_log_level)) {
    logLevel = static_cast<Level>(command_line::get_arg(vm, arg_log_level));
  }

  logManager.configure(buildLoggerConfiguration(logLevel, Common::ReplaceExtenstion(argv[0], ".log")));

  logger(INFO, CYAN) << CRYPTONOTE_NAME << " wallet " << PROJECT_VERSION_LONG << std::endl << std::endl;

  CryptoNote::Currency currency = CryptoNote::CurrencyBuilder(logManager).
    testnet(command_line::get_arg(vm, arg_testnet)).currency();

  if (command_line::has_arg(vm, Tools::wallet_rpc_server::arg_rpc_bind_port)) {
    //runs wallet with rpc interface
    if (!command_line::has_arg(vm, arg_wallet_file)) {
      logger(ERROR, BRIGHT_RED) << "Wallet file not set.";
      return 1;
    }

    if (!command_line::has_arg(vm, arg_daemon_address)) {
      logger(ERROR, BRIGHT_RED) << "Daemon address not set.";
      return 1;
    }

    if (!command_line::has_arg(vm, arg_password)) {
      logger(ERROR, BRIGHT_RED) << "Wallet password not set.";
      return 1;
    }

    std::string wallet_file = command_line::get_arg(vm, arg_wallet_file);
    std::string wallet_password = command_line::get_arg(vm, arg_password);
    std::string daemon_address = command_line::get_arg(vm, arg_daemon_address);
    std::string daemon_host = command_line::get_arg(vm, arg_daemon_host);
    uint16_t daemon_port = command_line::get_arg(vm, arg_daemon_port);
    if (daemon_host.empty())
      daemon_host = "localhost";
    if (!daemon_port)
      daemon_port = RPC_DEFAULT_PORT;

    if (!daemon_address.empty()) {
      if (!parseUrlAddress(daemon_address, daemon_host, daemon_port)) {
        logger(ERROR, BRIGHT_RED) << "failed to parse daemon address: " << daemon_address;
        return 1;
      }
    }

    std::unique_ptr<INode> node(new NodeRpcProxy(daemon_host, daemon_port));

    std::promise<std::error_code> errorPromise;
    std::future<std::error_code> error = errorPromise.get_future();
    auto callback = [&errorPromise](std::error_code e) {errorPromise.set_value(e); };
    node->init(callback);
    if (error.get()) {
      logger(ERROR, BRIGHT_RED) << ("failed to init NodeRPCProxy");
      return 1;
    }

    std::unique_ptr<IWalletLegacy> wallet(new WalletLegacy(currency, *node.get()));

    std::string walletFileName;
    try  {
      walletFileName = ::tryToOpenWalletOrLoadKeysOrThrow(logger, wallet, wallet_file, wallet_password);

      logger(INFO) << "available balance: " << currency.formatAmount(wallet->actualBalance()) <<
      ", locked amount: " << currency.formatAmount(wallet->pendingBalance());

      logger(INFO, BRIGHT_GREEN) << "Loaded ok";
    } catch (const std::exception& e)  {
      logger(ERROR, BRIGHT_RED) << "Wallet initialize failed: " << e.what();
      return 1;
    }

    Tools::wallet_rpc_server wrpc(dispatcher, logManager, *wallet, *node, currency, walletFileName);

    if (!wrpc.init(vm)) {
      logger(ERROR, BRIGHT_RED) << "Failed to initialize wallet rpc server";
      return 1;
    }

    Tools::SignalHandler::install([&wrpc, &wallet] {
      wrpc.send_stop_signal();
    });

    logger(INFO) << "Starting wallet rpc server";
    wrpc.run();
    logger(INFO) << "Stopped wallet rpc server";
    
    try {
      logger(INFO) << "Storing wallet...";
      CryptoNote::WalletHelper::storeWallet(*wallet, walletFileName);
      logger(INFO, BRIGHT_GREEN) << "Stored ok";
    } catch (const std::exception& e) {
      logger(ERROR, BRIGHT_RED) << "Failed to store wallet: " << e.what();
      return 1;
    }
  } else {
    //runs wallet with console interface
    CryptoNote::simple_wallet wal(dispatcher, currency, logManager);
    
    if (!wal.init(vm)) {
      logger(ERROR, BRIGHT_RED) << "Failed to initialize wallet"; 
      return 1; 
    }

    std::vector<std::string> command = command_line::get_arg(vm, arg_command);
    if (!command.empty())
      wal.process_command(command);

    Tools::SignalHandler::install([&wal] {
      wal.stop();
    });
    
    wal.run();

    if (!wal.deinit()) {
      logger(ERROR, BRIGHT_RED) << "Failed to close wallet";
    } else {
      logger(INFO) << "Wallet closed";
    }
  }
  return 1;
  //CATCH_ENTRY_L0("main", 1);
}
