# Goldo Daemon's JSON RPC API

## GETBLOCKCOUNT

Returns current chain height.

### getblockcount input:
```
{
	"jsonrpc": "2.0",
	"id" : "test",
	"method": "getblockcount",
		"params": {}
}
```
### getblockcount output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"count": 23520,
		"status": "OK"
	}
}
```

## GETBLOCKHASH

Returns block hash by its height.

### getblockhash input:
```
{
	"jsonrpc": "2.0",
	"id": "test",
	"method": "getblockhash",
	"params": [12345]
}
```

### getblockhash output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": "21eac691daa478dbbc3f941db0c8d5213db37035307258a164e0aa167c8b93ee"
}
```

## GETBLOCKTEMPLATE

Returns blocktemplate with an empty “hole” for nonce.

### getblocktemplate input:
```
{
	"jsonrpc": "2.0",
	"id" : "test",
	"method": "getblocktemplate",
	"params": {
		"reserve_size": 200,
		"wallet_address": "Ladpu63NXAQPuTzFNPRcSqS8CVsLM8T2w11uMjwPCXmkGc9bnHv6pY5hmreainiR9gKUQ8pHXFh6ASUHj8bfjmrwS1Wq1Lv"
	}
}
```
### getblocktemplate output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"blocktemplate_blob": "0400b9d97c00be3a53ae5fc7bb757feb1e450b731bbea8974c8b1b75023d234ac6500100d4b1dad...0000",
		"difficulty": 28852180,
		"height": 23497,
		"reserved_offset": 203,
		"status": "OK"
	}
}
```

## GETCURRENCYID

Returns unique currency identifier.

### getcurrencyid input:
```
{
	"jsonrpc": "2.0",
	"id" : "test",
	"method": "getcurrencyid"
}
```
### getcurrencyid output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"currency_id_blob": "f7657c92a306c22f1546beb2d1819c9b85696d1ba35dfa5ec423276462e25c86"
	}
}
```

## SUBMITBLOCK

Submits mined block.

### submitblock input:
```
{
	"jsonrpc": "2.0",
	"id" : "test",
	"method": "submitblock",
	"params": ["0100b...."]
}
```

### submitblock output:
```
{
	"error": {
		"code": -6,
		"message": "Wrong block blob"
	},
	"id": "test",
	"jsonrpc": "2.0"
}
```

## GETLASTBLOCKHEADER

Returns last block header.

### getlastblockheader input:
```
{
	"jsonrpc": "2.0",
	"id" : "test",
	"method": "getlastblockheader"
}
```
### getlastblockheader output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"block_header": {
			"depth": 0,
			"difficulty": 28895377,
			"hash": "74fefefaac01751f22ff8259e22f5021577f91767d66919d5e457d17917a7e49",
			"height": 23500,
			"major_version": 4,
			"minor_version": 0,
			"nonce": 1861908926,
			"orphan_status": false,
			"prev_hash": "74ace03a58e091c33886f6160cca71fc66610285d90cef74b78716a4da05bbb3",
			"reward": 200000000,
			"timestamp": 1511430619
		},
		"status": "OK"
	}
}
```

## GETBLOCKHEADERBYHASH

Returns last block header by given hash.

### getblockheaderbyhash input:
```
{
	"jsonrpc": "2.0",
	"id" : "test",
	"method": "getblockheaderbyhash",
	"params": {
		"hash" : "21eac691daa478dbbc3f941db0c8d5213db37035307258a164e0aa167c8b93ee"
	}
}
```
### getblockheaderbyhash output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"block_header": {
			"depth": 11157,
			"difficulty": 12180680,
			"hash": "21eac691daa478dbbc3f941db0c8d5213db37035307258a164e0aa167c8b93ee",
			"height": 12345,
			"major_version": 4,
			"minor_version": 0,
			"nonce": 731295514,
			"orphan_status": false,
			"prev_hash": "cc1b12a7fa3de8df11fb15c37a75d2f969d3535936afcd605369ace59721cd96",
			"reward": 100000000,
			"timestamp": 1510151512
		},
		"status": "OK"
	}
}
```

## GETBLOCKHEADERBYHEIGHT

Returns block header by given block height.

### getblockheaderbyheight input:
```
{
	"jsonrpc": "2.0",
	"id" : "test",
	"method": "getblockheaderbyheight",
	"params": {
		"height" : 12345
	}
}
```
### getblockheaderbyheight output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"block_header": {
			"depth": 11156,
			"difficulty": 12180680,
			"hash": "21eac691daa478dbbc3f941db0c8d5213db37035307258a164e0aa167c8b93ee",
			"height": 12345,
			"major_version": 4,
			"minor_version": 0,
			"nonce": 731295514,
			"orphan_status": false,
			"prev_hash": "cc1b12a7fa3de8df11fb15c37a75d2f969d3535936afcd605369ace59721cd96",
			"reward": 100000000,
			"timestamp": 1510151512
		},
		"status": "OK"
	}
}
```
## BLOCKS_LIST_JSON

Returns last 30 blocks info by given block height.

### blocks_list_json input:
```
{
	"jsonrpc": "2.0",
	"id": "test",
	"method": "blocks_list_json",
	"params": {
		"height": 12345
	}
}
```
### blocks_list_json output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"blocks": [{
			"cumul_size": 198,
			"difficulty": 12180680,
			"hash": "21eac691daa478dbbc3f941db0c8d5213db37035307258a164e0aa167c8b93ee",
			"height": 12345,
			"timestamp": 1510151512,
			"tx_count": 1
		},

		*****
		
		{
			"cumul_size": 198,
			"difficulty": 11143280,
			"hash": "969fb7516d5dd1f3acc88b49e1fdf5282c36d8c09fed2df89f08ef400c314ca9",
			"height": 12315,
			"timestamp": 1510148973,
			"tx_count": 1
		}],
		"status": "OK"
	}
}
```

## BLOCK_JSON

Returns block details by given block hash.

### block_json input:
```
{
	"jsonrpc": "2.0",
	"id": "test",
	"method": "block_json",
	"params": {
		"hash": "eec31658c8fea93ee4e6ff8f7df224c841988e32ea8b0ab066f8a23c66ecb427"
	}
}
```
### block_json output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"block": {
			"alreadyGeneratedCoins": "101953400000000",
			"alreadyGeneratedTransactions": 38118,
			"baseReward": 10000000000000,
			"blockSize": 16942,
			"depth": 21,
			"difficulty": 1214521,
			"effectiveSizeMedian": 140701365871406,
			"hash": "eec31658c8fea93ee4e6ff8f7df224c841988e32ea8b0ab066f8a23c66ecb427",
			"height": 38040,
			"major_version": 4,
			"minor_version": 0,
			"nonce": 1408307607,
			"orphan_status": true,
			"penalty": 0.0,
			"prev_hash": "7ebd8f5f77b1ea3d555ef3dc09c2c6b5873402f5b57e91b591fa2487ddd5f621",
			"reward": 100000001,
			"sizeMedian": 81,
			"timestamp": 1511786527,
			"totalFeeAmount": 1,
			"transactions": [{
				"amount_out": 100000001,
				"fee": 0,
				"hash": "756624eb27c53e373e5360f8d4b1c43b5eb4f01fa5b675d8b55fa7a5c5938279",
				"size": 115
			},
			{
				"amount_out": 10009099899,
				"fee": 1,
				"hash": "8ce7157a2fbbf6e4a80d61fe7140860871f62705e36608420b487c8b9ef183e1",
				"size": 16676
			}],
			"transactionsCumulativeSize": 16791
		},
		"status": "OK"
	}
}
*****
{
	"error": {
		"code": -5,
		"message": "Internal error: can't get block by hash. Hash = 8ce7157a2fbbf6e4a80d61fe7140860871f62705e36608420b487c8b9ef183e1."
	},
	"id": "test",
	"jsonrpc": "2.0"
}
```

## TRANSACTION_JSON

Returns transaction details by given hash.

### transaction_json input:
```
{
	"jsonrpc": "2.0",
	"id": "test",
	"method": "transaction_json",
	"params": {
		"hash": "8ce7157a2fbbf6e4a80d61fe7140860871f62705e36608420b487c8b9ef183e1"
	}
}
```
### transaction_json output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"block": {
			"cumul_size": 1538,
			"difficulty": 381697352304,
			"hash": "366753e100a4a8ea94cc7271aef91c59ef68007a65b9b8d9b979ef30f8cee140",
			"height": 38355,
			"timestamp": 1511792593,
			"tx_count": 2
		},
		"status": "OK",
		"tx": {
			"": "9afae5edddb58eb4e19c9b6b2fd4a4378094eb1b7d46817617d6d56d4a4fe0074c807a344431418860d8fa896b7254b6b41a95123ff4ecf2f6717a524387ce0a",
			"extra": "01255d033112908b41a1127ce1b0b71474ffe54a2ced24e5868d1b94f943dea9f3",
			"unlock_time": 0,
			"version": 1,
			"vin": [{
				"type": "02",
				"value": {
					"amount": 100000000,
					"k_image": "c8e81be49be2124af1d4d901f704bb5ef8090cd861edeb88a058b9e5f3306e87",
					"key_offsets": [3661, 29261]
				}
			},
			{
				"type": "02",
				"value": {
					"amount": 90000,
					"k_image": "e5586c926436e66ba719c7f5ff0f86e90c8809a0efb094bf24c3f534b6d31a34",
					"key_offsets": [0, 54]
				}
			},
			{
				"type": "02",
				"value": {
					"amount": 100000000,
					"k_image": "765010a1b81f898fabbfe7f19e5442def117fc408620f173db9056596c2c85c0",
					"key_offsets": [858, 30898]
				}
			},
			{
				"type": "02",
				"value": {
					"amount": 100000000,
					"k_image": "a4b059a6713e8fa8b5ead063fbc97036884f58a53ae811cbee9d1977ddf3cc6d",
					"key_offsets": [14582, 18070]
				}
			},
			{
				"type": "02",
				"value": {
					"amount": 100000000,
					"k_image": "68fa63567baa23993680b70a88c3df3f8383d105ad722947198d2e46246efba7",
					"key_offsets": [4170, 28179]
				}
			},
			{
				"type": "02",
				"value": {
					"amount": 100000000,
					"k_image": "d51ccbead66452e299ead9a1ee09a307b13711b6e5e82cc99d1ae67f7696ff69",
					"key_offsets": [1987, 31042]
				}
			}],
			"vout": [{
				"amount": 9,
				"target": {
					"data": {
						"key": "36a37f16ef559bfcb3d5906dc4e0859f1ab8a0cc770302fc5e2629f2c8ad2bd9"
					},
					"type": "02"
				}
			},
			{
				"amount": 90,
				"target": {
					"data": {
						"key": "e5e88a4dda134166aa5a1e3856ee620661e22fb69b7944bec60f258762edaef6"
					},
					"type": "02"
				}
			},
			{
				"amount": 900,
				"target": {
					"data": {
						"key": "4e52717bfca7863fa7e01fe253738a7b3666b0e4a7ac9620e57865e2e658891d"
					},
					"type": "02"
				}
			},
			{
				"amount": 9000,
				"target": {
					"data": {
						"key": "0c4996afdb0b3c89ab66d6d4ec39781bba67e4b95e1bfb2bff9c4e12a48827eb"
					},
					"type": "02"
				}
			},
			{
				"amount": 80000,
				"target": {
					"data": {
						"key": "eaabd924cd10e438daa129db9842889c37d769ffe5a111d4a0bc8076f11cd2dc"
					},
					"type": "02"
				}
			},
			{
				"amount": 500000000,
				"target": {
					"data": {
						"key": "2631e51765e35c9b06ffb097c3997be0676f38cdb611bfdd1864fd009ebc2599"
					},
					"type": "02"
				}
			}]
		},
		"txDetails": {
			"amount_out": 500089999,
			"fee": 1,
			"hash": "a8da678bfb714c2120e56765e62e64773950df4214243f966c98bf640cc9b369",
			"mixin": 2,
			"paymentId": "",
			"size": 1272
		}
	}
}
*****
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"block": {
			"cumul_size": 200,
			"difficulty": 381697350960,
			"hash": "bcd0d87515e099b95cb66dfe5c823dceaa9cb9687a951e873569b1b60209bd86",
			"height": 38303,
			"timestamp": 1511791623,
			"tx_count": 1
		},
		"status": "OK",
		"tx": {
			"extra": "0122369e991fac37ef5b8efaae4e3e689b8d2316b09ccc880a5a17863b12cce82f",
			"unlock_time": 38321,
			"version": 1,
			"vin": [{
				"type": "ff",
				"value": {
					"height": 38303
				}
			}],
			"vout": [{
				"amount": 200000000,
				"target": {
					"data": {
						"key": "8fad9916aa6b99433f97a5dec1aad308ef96f4778347c68aed4716dbbd136ed9"
					},
					"type": "02"
				}
			}]
		},
		"txDetails": {
			"amount_out": 200000000,
			"fee": 0,
			"hash": "387c60428f4b62c800ad111bac6e4e72ef4e27e712fee7f3c50447d415c81c4b",
			"mixin": 0,
			"paymentId": "",
			"size": 81
		}
	}
}
*****
{
	"error": {
		"code": -1,
		"message": "transaction wasn't found. Hash = eec31658c8fea93ee4e6ff8f7df224c841988e32ea8b0ab066f8a23c66ecb427."
	},
	"id": "test",
	"jsonrpc": "2.0"
}
```

## TRANSACTIONS_POOL_JSON

Returns current mempool transactions.

### transactions_pool_json input:
```
{
	"jsonrpc": "2.0",
	"id": "test",
	"method": "transactions_pool_json",
	"params": {}
}
```

### transactions_pool_json output:
```
{
	"id": "test",
	"jsonrpc": "2.0",
	"result": {
		"status": "OK",
		"transactions": [{
			"amount_out": 10090999897,
			"fee": 1,
			"hash": "c1332894011b59b0f3593c7c0653805ac4c6f08922d33d59a5d5932f77fb4aff",
			"size": 17528
		}]
	}
}
```

