// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2014-2017 XDN-project developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "rulez.h"

namespace CryptoNote
{
//	const static boost::uuids::uuid CRYPTONOTE_NETWORK = { { 0x4E, 0x6F, 0x73, 0x63, 0x65, 0x20, 0x74, 0x65, 0x20, 0x69, 0x70, 0x73, 0x75, 0x6D, 0x20, 0x20 } };
	
#ifdef SOLDO_CODEBASE
	//CRYPTONOTE_NETWORK = 24d91a1b-3aa0-6ddf-3eab-e0b4fb48f9e9
	const static boost::uuids::uuid CRYPTONOTE_NETWORK = { { 0x24, 0xD9, 0x1A, 0x1B, 0x3A, 0xA0, 0x6D, 0xDF, 0x3E, 0xAB, 0xE0, 0xB4, 0xFB, 0x48, 0xF9, 0xE9 } };
#endif
	
#ifdef GOLDO_CODEBASE
	//CRYPTONOTE_NETWORK = 24d91a1b-3aa0-6ddf-7bab-e0b4fb48f9e9
	const static boost::uuids::uuid CRYPTONOTE_NETWORK = { { 0x24, 0xD9, 0x1A, 0x1B, 0x3A, 0xA0, 0x6D, 0xDF, 0x7B, 0xAB, 0xE0, 0xB4, 0xFB, 0x48, 0xF9, 0xE9 } };
#endif
	
}
