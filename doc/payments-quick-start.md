# Payments Quick Start Guide

## Basic settings

First of all you have to set basic settings for paymets application launch.

You should create config file like below:

```
server-root=C:\Bin\BIT\SLD\
data-dir=B:\BLOCKS\SLD
container-file=Y:\Wallets\SLD\sld-aaa.container
container-password=My-Long-And-Boring-Password-ABCD
bind-port=33717
bind-address=0.0.0.0
p2p-bind-port=33719
log-file=B:\BLOCKS\sldp.log
hide-my-port=1
local=1
```

### Full list of settings and application params is below:
```
Common Options:
  -c [ --config ] arg                   configuration file
  -h [ --help ]                         produce this help message and exit
  --local                               start with local node (remote is default)
  --testnet                             testnet mode
  --data-dir arg (=C:\Users\Administrator\AppData\Roaming/sld) Specify data directory
  --bind-address arg (=0.0.0.0)         payment service bind address
  --bind-port arg (=8070)               payment service bind port
  --rpc-user arg                        username to use the payment service. If authorization is not required, leave it empty
  --rpc-password arg                    password to use the payment service. If authorization is not required, leave it empty
  -w [ --container-file ] arg           container file
  -p [ --container-password ] arg       container password
  -g [ --generate-container ]           generate new container file with one wallet and exit
  -d [ --daemon ]                       run as daemon in Unix or as service in  Windows
  --register-service                    register service and exit (Windows only)
  --unregister-service                  unregister service and exit (Windows only)
  -l [ --log-file ] arg                 log file
  --server-root arg                     server root. The service will use it as working directory. Don't set it if don't want to change it
  --log-level arg                       log level
  --address                             print wallet addresses and exit

Remote Node Options:
  --daemon-address arg (=localhost)     daemon address
  --daemon-port arg (=8081)             daemon port

Local Node Options:
  --p2p-bind-ip arg (=0.0.0.0)          Interface for p2p network protocol
  --p2p-bind-port arg (=33711)          Port for p2p network protocol
  --p2p-external-port arg (=0)          External port for p2p network protocol  (if port forwarding used with NAT)
  --allow-local-ip                      Allow local ip add to peer list, mostly in debug purposes
  --add-peer arg                        Manually add peer to local peerlist
  --add-priority-node arg               Specify list of peers to connect to and attempt to keep the connection open
  --add-exclusive-node arg              Specify list of peers to connect to  only. If this option is given the options add-priority-node and seed-node are ignored
  --seed-node arg                       Connect to a node to retrieve peer addresses, and disconnect
  --hide-my-port                        Do not announce yourself as peerlist candidate
```

## Generating Container

Next you need to generate container to hold your wallets.
Run command like below to get your container.

```
sldp -c C:\Bin\BIT\SLD\sld-galapagos.ini -w C:\Bin\BIT\SLD\sld-aaa.container -p My-Long-And-Boring-Password-ABCDE -g
```

Your container will be created and application will terminate.

## Start Payment application

Remove last param *-g* from previous example and start Payment Application again.
Payment application will start and ready to service your requests.

## Working with Payment application

You have to HTTP POST requests in JSON format using http://127.0.0.1:33717/json_rpc endpoint.

Change port number to value you set at _bind-port_ param in config file.

If you use Payment application at remote node you have to change IP-address as well.

### List of available commands

	reset
	createAddress
	deleteAddress
	getSpendKeys
	getBalance
	getBlockHashes
	getTransactionHashes
	getTransactions
	getUnconfirmedTransactionHashes
	getTransaction
	sendTransaction
	createDelayedTransaction
	getDelayedTransactionHashes
	deleteDelayedTransaction
	sendDelayedTransaction
	getViewKey
	getStatus
	getAddresses

### Checking status

To make sure all settings is correct try to get current status.

#### Request
```
{
	"params": {},
	"jsonrpc": "2.0",
	"id": "test_status",
	"method": "getStatus"
}
```
#### Responce
```
{
	"id": "test_status",
	"jsonrpc": "2.0",
	"result": {
		"blockCount": 275984,
		"knownBlockCount": 275984,
		"lastBlockHash": "08af2dbe459bd897e3cb1007af5d475f1dc694c4635abad138125b64d9da7b83",
		"peerCount": 16
	}
}
```
### Checking wallet balance

Your new container should have zero balance.

getBalance() method returns a balance for a specified address.

Please note: If address is not specified, returns a cumulative balance of all container addresses. 

#### Request
```
{
	"params": {}, 
	"jsonrpc": "2.0", 
	"id": "test_balance", 
	"method": "getBalance"
}
```
#### Responce
```
{
	"id": "test_balance",
	"jsonrpc": "2.0",
	"result": {
		"availableBalance": 0,
		"lockedAmount": 0
	}
}
```
### Get view key

Backup view key before run application in production.

#### Request
```
{
	"params": {},
	"jsonrpc": "2.0",
	"id": "test_view_key",
	"method": "getViewKey"
}
```
#### Responce
```
{
	"id": "test_view_key",
	"jsonrpc": "2.0",
	"result": {
		"viewSecretKey": "5caa5086ffc09e0f36a32d969f94584143a47078799c7e8d59b56f60222c9e0b"
	}
}
```
### List wallets

Your new container should have at least one default wallet.

#### Request
```
{
	"params": {},
	"jsonrpc": "2.0",
	"id": "test_addr",
	"method": "getAddresses"
}
```
#### Responce
```
{
	"id": "test_addr",
	"jsonrpc": "2.0",
	"result": {
		"addresses": 
		[
		"LfCxYeXHauBbWAnWfnJCVQiptRWLQ7QsTgRzmtuita6mbxUsrV6TbeY6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaToQZ4u4",
		"LisXTv9J2J6RgVbb53EPNde3TXxnSxfLQfuJSipRhYeiJE9aNvJB9Hi6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTnsWhVv"
		]
	}
}
```
### Get spend key

Backup spend key before run application in production as well.

#### Request
```
{
	"params": {"address": "LfCxYeXHauBbWAnWfnJCVQiptRWLQ7QsTgRzmtuita6mbxUsrV6TbeY6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaToQZ4u4"},
	"jsonrpc": "2.0",
	"id": "test_spend_key",
	"method": "getSpendKeys"
}
```
#### Responce
```
{
	"id": "test_spend_key",
	"jsonrpc": "2.0",
	"result": {
		"spendPublicKey": "88274aae9bde36ce47879ae52525f3fa0f21c62b93ac2aebbe0ab17c3656bad0",
		"spendSecretKey": "92dc12dee4ab24eeeee8826e4e3bd1315923a9c61c850c4500413e6b01d14203"
	}
}
```
### Create new wallet

Creating new wallet without params

#### Request
```
{
	"params": {},
	"jsonrpc": "2.0",
	"id": "test_create_address",
	"method": "createAddress"
}
```
#### Responce
```
{
	"id": "test_create_address",
	"jsonrpc": "2.0",
	"result": {
		"address": "LisXTv9J2J6RgVbb53EPNde3TXxnSxfLQfuJSipRhYeiJE9aNvJB9Hi6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTnsWhVv"
	}
}
```
### Delete wallet

Deleting existing wallet

#### Request
```
{
	"params": {"address": "LisXTv9J2J6RgVbb53EPNde3TXxnSxfLQfuJSipRhYeiJE9aNvJB9Hi6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTnsWhVv"},
	"jsonrpc": "2.0",
	"id": "test_delete_address",
	"method": "deleteAddress"
}
```
#### Responce
```
{
	"id": "test_delete_address",
	"jsonrpc": "2.0",
	"result": {
		
	}
}
```
### Get block hashes

List serial blocks from given height

#### Request
```
{
	"params": 
		{"firstBlockIndex": 999,"blockCount": 12},
	"jsonrpc": "2.0",
	"id": "test_get_blck_hash",
	"method": "getBlockHashes"
}
```
#### Responce
```
{
	"id": "test_get_blck_hash",
	"jsonrpc": "2.0",
	"result": {
		"blockHashes": [
		"bb58a5622afd4732454cf3ec1309ff84aba18433d7e9ce0e6bbdc1b319a604d2", 
		"e71a72d72446b61b70fb11bc4b9f5a9b4acbea772522876181aaab6cfeb9f8b3", 
		"f1a5811fd23e8cd677c209789e4c2fbce92b00fc16de9bebd9238f6493691129", 
		"3e8c7cee797e2a8fe0c11a63a7dec30e3fae820f5a27b7a3eba2600e5a7c7e01", 
		"3d847acc914b4e4b7376f94428f09ffa21789c798a0ff4a73b3a993841686ff4", 
		"13b1b13bf42ed4335f13e5a3c0b3d1fc4c917f1dc8b4ab7b75d46212ba759ba1", 
		"445d53ef6d3f5011ad18b38e3d730da0897eca0e38bab6b312eb68eb34f7bfa6", 
		"92420805181e3530463862b9bff9b8e74c8e44aff289ae4e864be6c3eb4155bb", 
		"d1323a4908a0cfe063b2a34bc56d80ceaf7687761a62d1ea3f86c38a5adfe521", 
		"59102d8bdc4ad593f2ca7b5f45b4895872c7b7c5772a883f5c01aa822e188287", 
		"0899792e7057ca0647937aeebf09f5a72c7f1b8d93e790ba61e6abb7d05359c9", 
		"429c99bcc2a4f05cfebd6d399a07962c092cb67125739ed75b033b082b71bffe"
		]
	}
}
```
### Get transaction hashes

getTransactionHashes() method returns an array of block and transaction hashes.

Transaction consists of transfers. 
Transfer is an amount-address pair. 
There could be several transfers in a single transaction.

#### Request

Note: if paymentId parameter is set, getTransactionHashes() method returns transaction hashes of transactions that contain specified payment_id. (in the set block range)

Note: if addresses parameter is set, getTransactionHashes() method returns transaction hashes of transactions that contain transfer from at least one of specified addresses.

Note: if both above mentioned parameters are set, getTransactionHashes() method returns transaction hashes of transactions that contain both specified payment_id and transfer from at least one of specified addresses. 

```
{
	"params": 
		{"firstBlockIndex": 276930,"blockCount": 12},
	"jsonrpc": "2.0",
	"id": "test_get_tx_hashs",
	"method": "getTransactionHashes"
}
```
#### Responce
Array that contains:

    blockHash - string - hash of the block which contains transaction hashes
    transactionHashes - array - array of strings, where each string is a transaction hash 
```
{
	"id": "test_get_tx_hashs",
	"jsonrpc": "2.0",
	"result": {
		"items": [{
			"blockHash": "6433199365b938e36d984a4cd9e52081a439c3f089f7a829531c2c30844facf7",
			"transactionHashes": []
		},
		{
			"blockHash": "cd1281b8da0baf7086414596efb9104f46a49f35da70e3a13ce6136cc8c13d46",
			"transactionHashes": []
		},
		{
			"blockHash": "922624a1cce0a33ce02f8535d91222b4a2ab624435c8e508a9b848bd11fc1dd9",
			"transactionHashes": []
		},
		{
			"blockHash": "27785bd8da6db1e483dac7ae4bbe0f40e56e218c1fd5950fed0826861940b36c",
			"transactionHashes": ["6a740064adfff1a162ccff830f5a6fda0138bc6399ffbe5438e71c13f4e60a91"]
		},
		{
			"blockHash": "69be1e86bfa7b19f73990e61b46877186253709a3fe523d228a899e607b53457",
			"transactionHashes": []
		},
		{
			"blockHash": "95e4795a1ccd220c1b4aec270e5bfdf15daccf4f1c6850642c62c49dd5bb07b4",
			"transactionHashes": ["b9b7c0ea758c64e84e434224f5c2a7696f5b211e7d1d5f351c5f59539cfe9cef"]
		},
		{
			"blockHash": "2b0d693459e9e949da698be8ef2508753ade8491796088c295dbd63dc714aa63",
			"transactionHashes": []
		},
		{
			"blockHash": "2d79462e8f91d4f56909aa72aee2b59d9c4b47b6080a70608788b965cdb6f11c",
			"transactionHashes": []
		},
		{
			"blockHash": "4f4647bc8a8a2687d6cd5238779161fac8731ee3a8aab24fed88c7446eb959a0",
			"transactionHashes": []
		},
		{
			"blockHash": "cd38cf4f86d8c6550e3bc8a2c3ed5fac30df1aa9ff96462acebc36f313d88c76",
			"transactionHashes": ["8fce100b801b2dd5a1ce67bef9abf8ea88f89ae95f9684a437c4fd75f72cbf4c"]
		},
		{
			"blockHash": "eb35ba7b083906c9a01f0e315b44b967d97d6c3751f2c2bb969cdcd1018dfbfc",
			"transactionHashes": ["3e4f7ce375d14ef71957d72b2644ca5a2ca6c8dc672d29ff49a093fdd807a0a9"]
		},
		{
			"blockHash": "e6f341d2f534972d7da91f5b85d9571e3723b3fb2badecb2cc7b812a234b798b",
			"transactionHashes": []
		}]
	}
}
```
### Get transactions

getTransactions() method returns an array of block and transaction hashes.

Transaction consists of transfers. 
Transfer is an amount-address pair. 
There could be several transfers in a single transaction. 

#### Request

Note: if paymentId parameter is set, getTransactions() method returns transactions that contain specified payment_id. (in the set block range)

Note: if addresses parameter is set, getTransactions() method returns transactions that contain transfer from at least one of specified addresses.

Note: if both above mentioned parameters are set, getTransactions() method returns transactions that contain both specified payment_id and transfer from at least one of specified addresses. 

```
{
	"params": 
		{"firstBlockIndex": 280733,"blockCount": 3},
	"jsonrpc": "2.0",
	"id": "test_get_txs",
	"method": "getTransactions"
}
```
#### Responce

Array that contains:

	block_hash - string - hash of the block which contains a transaction
	transactions - array - contains
		transactionHash - string - hash of the transaction
		blockIndex - uint32 - number of the block that contains a transaction
		timestamp - uint64 - timestamp of the transaction
		isBase - boolean - shows if the transaction is a coinbase transaction or not
		unlockTime - uint64 - height of the block when transaction is going to be available for spending
		amount - int64 - amount of the transaction
		fee - uint64- transaction fee
		extra - string
		paymentId - string - payment_id of the transaction (optional)
		transfers - array - contains
			address - string
			amount - int64 

```
{
	"id": "test_get_txs",
	"jsonrpc": "2.0",
	"result": {
		"items": [{
			"blockHash": "7ce1ea6cfae3d394e28845de2b8f2c6e628ffbfbca2cb089f2cd96995cecf5a6",
			"transactions": []
		},
		{
			"blockHash": "de5c7264582105536e36489454327b0b469ed4dea12ebd63b2d59f3876905087",
			"transactions": []
		},
		{
			"blockHash": "91f2193fde685ddf6cc77920094b2685df35e4946a930c69d2e3be70ae17e68e",
			"transactions": [{
				"amount": 7200,
				"blockIndex": 280734,
				"extra": "022100000000000000000000000000000000000000000000000000000000000000005101186c7b19d613bdb9817e740a7bf87919ca7a3c6ebcd1746474f728d06f2abf67041552905208b35d3c21b87b183745c5a4e9290bfa40040415d62c44e318c16a973946092707498d8fbbfec742bd",
				"fee": 1,
				"isBase": false,
				"paymentId": "0000000000000000000000000000000000000000000000000000000000000051",
				"state": 0,
				"timestamp": 1516544268,
				"transactionHash": "1324afd42ed6f838a2cd64a26b3706ad3278d23dec930523e28d5311300d7b54",
				"transfers": [{
					"address": "LehUKzLxowc47shuT6RY1v7JjnEkanydzDTuQieNBHVyiUq7Py2hK5i6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTp2GpH2",
					"amount": 200,
					"type": 0
				},
				{
					"address": "LcRth5uo1Ks2Gh2h2jS4nCM4SzzxXTFZ8THNMykGQ923HZLtKh2CiDe6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTjzyK3P",
					"amount": 7000,
					"type": 0
				},
				{
					"address": "",
					"amount": -200000000,
					"type": 0
				},
				{
					"address": "",
					"amount": 199992799,
					"type": 0
				}],
				"unlockTime": 0
			}]
		}]
	}
}
```

### Get unconfirmed transaction hashes

getUnconfirmedTransactionHashes() method returns information about the current unconfirmed transaction pool or for a specified addresses.

Transaction consists of transfers. 
Transfer is an amount-address pair. 
There could be several transfers in a single transaction. 

#### Request

Note: if addresses parameter is set, getUnconfirmedTransactionHashes() method returns transactions that contain transfer from at least one of specified addresses. 

```
{
	"params": 
		{},
	"jsonrpc": "2.0",
	"id": "test_get_pending_tx_hashs",
	"method": "getUnconfirmedTransactionHashes"
}
```
#### Responce

Array of strings, where each string is a hash of an unconfirmed transaction

```
{
	"id": "test_get_pending_tx_hashs",
	"jsonrpc": "2.0",
	"result": {
		"transactionHashes": ["cf0847e37484baa0fb8caab8a2ad4b3f22ee9b4a4da949ad0072a85398f9059c"]
	}
}
```

### Get transaction

getTransaction() method returns information about a particular transaction.

Transaction consists of transfers. 
Transfer is an amount-address pair. 
There could be several transfers in a single transaction. 

#### Request

transactionHash is mandatory, contains hash of the requested transaction.

```
{
	"params": {
		"transactionHash": "1324afd42ed6f838a2cd64a26b3706ad3278d23dec930523e28d5311300d7b54"
	},
	"jsonrpc": "2.0",
	"id": "test_get_tx",
	"method": "getTransaction"
}
```
#### Responce

Contains:

    transactionHash - string - hash of the transaction
    blockIndex - uint32 - number of the block that contains a transaction (optional)
    timestamp - uint64 - timestamp of the transaction (optional)
    isBase - boolean - shows if the transaction is a coinbase transaction or not
    unlockTime - uint64 - height of the block when transaction is going to be available for spending
    amount - int64 - amount of the transaction
    fee - uint64- transaction fee
    extra - string - ?
    paymentId - string - payment_id of the transaction (optional)
    transfers - array - contains
        address - string
        amount - int64 
```
{
	"id": "test_get_tx",
	"jsonrpc": "2.0",
	"result": {
		"transaction": {
			"amount": 7200,
			"blockIndex": 280734,
			"extra": "022100000000000000000000000000000000000000000000000000000000000000005101186c7b19d613bdb9817e740a7bf87919ca7a3c6ebcd1746474f728d06f2abf67041552905208b35d3c21b87b183745c5a4e9290bfa40040415d62c44e318c16a973946092707498d8fbbfec742bd",
			"fee": 1,
			"isBase": false,
			"paymentId": "0000000000000000000000000000000000000000000000000000000000000051",
			"state": 0,
			"timestamp": 1516544268,
			"transactionHash": "1324afd42ed6f838a2cd64a26b3706ad3278d23dec930523e28d5311300d7b54",
			"transfers": [{
				"address": "LehUKzLxowc47shuT6RY1v7JjnEkanydzDTuQieNBHVyiUq7Py2hK5i6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTp2GpH2",
				"amount": 200,
				"type": 0
			},
			{
				"address": "LcRth5uo1Ks2Gh2h2jS4nCM4SzzxXTFZ8THNMykGQ923HZLtKh2CiDe6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTjzyK3P",
				"amount": 7000,
				"type": 0
			},
			{
				"address": "",
				"amount": -200000000,
				"type": 0
			},
			{
				"address": "",
				"amount": 199992799,
				"type": 0
			}],
			"unlockTime": 0
		}
	}
}
```
### Send transaction

sendTransaction() method allows you to send transaction to one or several addresses. Also, it allows you to use a payment_id for a transaction to a single address.

#### Request

Note: if container contains only 1 address, changeAddress field can be left empty and the change is going to be sent to this address

Note: if addresses field contains only 1 address, changeAddress can be left empty and the change is going to be sent to this address

Note: in the rest of the cases, changeAddress field is mandatory and must contain an address. 

```
{
	"params": {
		"anonymity": 3,
		"fee": 1,
		"unlockTime": 0,
		"paymentId": "0000000000000000000000000000000000000000000000000000000000000052",
		"addresses": [
			"LcRth5uo1Ks2Gh2h2jS4nCM4SzzxXTFZ8THNMykGQ923HZLtKh2CiDe6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTjzyK3P"
		],
		"transfers": [
			{"amount": 5555,"address": "LaA5LchrhoUSEnGyKv4n5AaTXi2GYc4fG2EHyBbSDc64cvjUHdjNXzKSswJ6GU9jpfHeRAeSHkEyL9MLFgs5QsrJUe8Rxdj"}
		],
		"changeAddress": "LcRth5uo1Ks2Gh2h2jS4nCM4SzzxXTFZ8THNMykGQ923HZLtKh2CiDe6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTjzyK3P"
	},
	"jsonrpc": "2.0",
	"id": "test_send_tx",
	"method": "sendTransaction"
}
```
#### Responce

Result is hash of the sent transaction.
 
```
{
	"id": "test_send_tx",
	"jsonrpc": "2.0",
	"result": {
		"transactionHash": "c27d8e3e1c7c8a4c21fa122654d921d8edafa74a4f66d9935bc83d35341f45f3"
	}
}
```
### Create delayed transaction

createDelayedTransaction() method creates a delayed transaction. Such transactions are not sent into the network automatically and should be pushed using sendDelayedTransaction method 

#### Request


Note: if container contains only 1 address, changeAddress field can be left empty and the change is going to be sent to this address

Note: if addresses field contains only 1 address, changeAddress can be left empty and the change is going to be sent to this address

Note: in the rest of the cases, changeAddress field is mandatory and must contain an address.

Note: outputs that were used for this transactions will be locked until the transaction is sent or cancelled

```
{
	"params": {
		"anonymity": 3,
		"fee": 1,
		"unlockTime": 0,
		"paymentId": "0000000000000000000000000000000000000000000000000000000000000052",
		"addresses": [
			"LcRth5uo1Ks2Gh2h2jS4nCM4SzzxXTFZ8THNMykGQ923HZLtKh2CiDe6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTjzyK3P"
		],
		"transfers": [
			{"amount": 333,"address": "LaA5LchrhoUSEnGyKv4n5AaTXi2GYc4fG2EHyBbSDc64cvjUHdjNXzKSswJ6GU9jpfHeRAeSHkEyL9MLFgs5QsrJUe8Rxdj"}
		],
		"changeAddress": "LcRth5uo1Ks2Gh2h2jS4nCM4SzzxXTFZ8THNMykGQ923HZLtKh2CiDe6AXQsSPsy2SAL8hGFzzJRmPkytpevLKQaTjzyK3P"
	},
	"jsonrpc": "2.0",
	"id": "test_create_dd_tx",
	"method": "createDelayedTransaction"
}
```
#### Responce

Result is hash of prepared transaction.
 
```
{
	"id": "test_create_dd_tx",
	"jsonrpc": "2.0",
	"result": {
		"transactionHash": "2de0a867705fa4a667239aabe510fffd2624a6f5a578625b3da0165234324429"
	}
}
```

### Get delayed transaction hashes 

getDelayedTransactionHashes() method returns hashes of delayed transactions. 
#### Request
No input.
```
{
	"params": {},
	"jsonrpc": "2.0",
	"id": "test_get_dd_tx_hashes",
	"method": "getDelayedTransactionHashes"
}
```
#### Responce
Array of strings, where each string is delayed a transaction hash.
```
{
	"id": "test_get_dd_tx_hashes",
	"jsonrpc": "2.0",
	"result": {
		"transactionHashes": ["2de0a867705fa4a667239aabe510fffd2624a6f5a578625b3da0165234324429"]
	}
}
```
### Send delayed transaction

sendDelayedTransaction() method sends a specified delayed transaction. 
 
#### Request
transactionHash is an existing delayed transaction.
```
{
	"params": {"transactionHash": "2de0a867705fa4a667239aabe510fffd2624a6f5a578625b3da0165234324429"},
	"jsonrpc": "2.0",
	"id": "test_send_dd_tx",
	"method": "sendDelayedTransaction"
}
```
#### Responce
In case of success returns an empty JSON object.
```
{
	"id": "test_send_dd_tx",
	"jsonrpc": "2.0",
	"result": {}
}
```

### Delete delayed transaction

deleteDelayedTransaction() method deletes a specified delayed transaction. 
 
#### Request
transactionHashis an existing delayed transaction .
```
{
	"params": {"transactionHash": "2de0a867705fa4a667239aabe510fffd2624a6f5a578625b3da0165234324429"},
	"jsonrpc": "2.0",
	"id": "test_del_dd_tx",
	"method": "deleteDelayedTransaction"
}
```
#### Responce
In case of success returns an empty JSON object.
```
{
	"id": "test_del_dd_tx",
	"jsonrpc": "2.0",
	"result": {}
}
```

### Reset wallets

Sometimes you need to reset wallets.
reset() method allows you to re-sync your wallet. 

#### Request
If the view_secret_key was not pointed out reset() methods resets the wallet and re-syncs it.
If the view_secret_key argument was pointed out reset() method substitutes the existing wallet with a new one with a specified view_secret_key and creates an address for it.

```
{
	"params": {
		"viewSecretKey": "5caa5086ffc09e0f36a32d969f94584143a47078799c7e8d59b56f60222c9e0b"
	},
	"jsonrpc": "2.0",
	"id": "test_reset",
	"method": "reset"
}
```
#### Responce
No output in case of success. 
```
{
	"id": "test_reset",
	"jsonrpc": "2.0",
	"result": {}
}
```

### Data Structures

#### Reset 
##### Request 
 * string viewSecretKey;
##### Response 

#### GetViewKey 
##### Request 
##### Response 
 * string viewSecretKey;

#### GetStatus 
##### Request 
##### Response 
 * integer blockCount;
 * integer knownBlockCount;
 * string lastBlockHash;
 * integer peerCount;

#### GetAddresses 
##### Request 
##### Response 
 * array<string> addresses;

#### CreateAddress 
##### Request 
 * string spendSecretKey;
 * string spendPublicKey;
##### Response 
 * string address;

#### DeleteAddress 
##### Request 
 * string address;
##### Response 
    
#### GetSpendKeys 
##### Request 
 * string address;
##### Response 
 * string spendSecretKey;
 * string spendPublicKey;

#### GetBalance 
##### Request 
 * string address;
##### Response 
 * integer availableBalance;
 * integer lockedAmount;

#### GetBlockHashes 
##### Request 
 * integer firstBlockIndex;
 * integer blockCount;
##### Response 
 * array<string> blockHashes;

#### GetTransactionHashes 
##### Request 
 * array<string> addresses;
 * string blockHash;
 * integer firstBlockIndex;
 * integer blockCount;
 * string paymentId;
##### Response 
 * array<TransactionHashesInBlockRpcInfo> items;

#### GetTransaction 
##### Request 
 * string transactionHash;
##### Response 
 * TransactionRpcInfo transaction;
###### TransactionsInBlockRpcInfo 
 * string blockHash;
 * array<TransactionRpcInfo> transactions;

#### GetTransactions 
##### Request 
 * array<string> addresses;
 * string blockHash;
 * integer firstBlockIndex;
 * integer blockCount;
 * string paymentId;
##### Response 
 * array<TransactionsInBlockRpcInfo> items;

#### GetUnconfirmedTransactionHashes 
##### Request 
 * array<string> addresses;
##### Response 
 * array<string> transactionHashes;

#### SendTransaction 
##### Request 
 * array<string> sourceAddresses;
 * array<WalletRpcOrder> transfers;
 * string changeAddress;
 * integer fee;
 * integer anonymity;
 * string extra;
 * string paymentId;
 * integer unlockTime;
##### Response 
 * string transactionHash;

#### CreateDelayedTransaction 
##### Request 
 * array<string> addresses;
 * array<WalletRpcOrder> transfers;
 * string changeAddress;
 * integer fee;
 * integer anonymity;
 * string extra;
 * string paymentId;
 * integer unlockTime;
##### Response 
 * string transactionHash;

#### GetDelayedTransactionHashes 
##### Request 
##### Response 
 * array<string> transactionHashes;

#### DeleteDelayedTransaction 
##### Request 
 * string transactionHash;
##### Response 

#### SendDelayedTransaction 
##### Request 
 * string transactionHash;
##### Response 
    
###### WalletRpcOrder 
 * string address;
 * integer amount;
###### TransactionHashesInBlockRpcInfo 
 * string blockHash;
 * array<string> transactionHashes;
###### TransferRpcInfo 
 * integer type;
 * string address;
 * integer amount;
###### TransactionRpcInfo 
 * integer state;
 * string transactionHash;
 * integer blockIndex;
 * integer timestamp;
 * bool isBase;
 * integer unlockTime;
 * integer amount;
 * integer fee;
 * array<TransferRpcInfo> transfers;
 * string extra;
 * string paymentId;
 