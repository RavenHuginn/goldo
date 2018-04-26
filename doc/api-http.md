# Goldo Daemon's HTTP API

## GETINFO

Returns current chain height.

### input

/getinfo 

### output 
```
{
	"alt_blocks_count": 0,
	"difficulty": 26246906,
	"full_deposit_amount": 0,
	"full_deposit_interest": 0,
	"grey_peerlist_size": 143,
	"height": 23549,
	"incoming_connections_count": 0,
	"last_known_block_index": 23548,
	"outgoing_connections_count": 9,
	"status": "OK",
	"tx_count": 60,
	"tx_pool_size": 0,
	"white_peerlist_size": 35
}
```
## GETHEIGHT

Returns list of known block ids.

### input

/getheight

### output 
```
{
	"alt_blocks_count": 0,
	"difficulty": 26246906,
	"full_deposit_amount": 0,
	"full_deposit_interest": 0,
	"grey_peerlist_size": 143,
	"height": 23549,
	"incoming_connections_count": 0,
	"last_known_block_index": 23548,
	"outgoing_connections_count": 9,
	"status": "OK",
	"tx_count": 60,
	"tx_pool_size": 0,
	"white_peerlist_size": 35
}
```

## START_MINING

Starts mining threads.

### input

/start_mining
```
{
	"miner_address": "Ladpu63NXAQPuTzFNPRcSqS8CVsLM8T2w11uMjwPCXmkGc9bnHv6pY5hmreainiR9gKUQ8pHXFh6ASUHj8bfjmrwS1Wq1Lv",
	"threads_count": 2
}
```

### output 
```
{
	"status": "OK"
}
```

## STOP_MINING

Stops mining threads.

### input

/stop_mining

### output 
```
{
	"status": "OK"
}
```

## STOP_DAEMON

Stops the daemon app.

### input

/stop_daemon

### output 
```
{
	"status": "OK"
}
```

## GETTRANSACTIONS

Returns transactions as serialized blobs.

### input

/gettransactions

```
{
	"txs_hashes": [
		"16227b19645572f289f18ad666f80793e10ddd304acf6e08c8b35a04dbc370cb",
		"18ad0c5de471132bb1f1c4c09e1884ec594458f41d8db78c0c4450b9d3eeadc8"
		]
}
```
### output 
```
{
	"missed_tx": [],
	"status": "OK",
	"txs_as_hex": ["01d6b80101ffc4b8010180c2d72f0288c39d3dc9d7ad9669d1c3e3b1d5c0f79d276f6f14b6142b3e7483ca4805402c210176acddfd6cc13f1b87180ebec6b732d6af50130ca75064f3dcc4bbfac5ff242b",
	"01000c028095f...c1eaa6780301"]
}
//////
{
	"missed_tx": ["08ad0c5de471132bb1f1c4c09e1884ec594458f41d8db78c0c4450b9d3eeadc8"],
	"status": "OK",
	"txs_as_hex": ["01d6b80101ffc4b8010180c2d72f0288c39d3dc9d7ad9669d1c3e3b1d5c0f79d276f6f14b6142b3e7483ca4805402c210176acddfd6cc13f1b87180ebec6b732d6af50130ca75064f3dcc4bbfac5ff242b"]
}
```

## SENDRAWTRANSACTION

Send transaction to the network.

### input

/sendrawtransaction

```
{
	"txs_as_hex": [
		"01d6b80101ffc4b8010180c2d72f0288c39d3dc9d7ad9669d1c3e3b1d5c0f79d276f6f14b6142b3e7483ca4805402c210176acddfd6cc13f1b87180ebec6b732d6af50130ca75064f3dcc4bbfac5ff242b"
		]
}
```

### output 
```
{
	"status": "Failed"
}
```
