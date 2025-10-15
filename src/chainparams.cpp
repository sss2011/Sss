// SPDX-License-Identifier: MIT

// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2015-2025 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "chainparams.h"
#include "consensus/merkle.h"
#include "key_io.h"
#include "main.h"
#include "crypto/equihash.h"

#include "tinyformat.h"
#include "util/system.h"
#include "util/strencodings.h"

#include <assert.h>
#include <optional>
#include <variant>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

// Initial network parameters, genesis block, upgrades, etc

// =========================== Main Network Parameters =====================

class CMainParams : public CChainParams {
public:
    CMainParams() {
        keyConstants.strNetworkID = "shifocoin";
        strCurrencyUnits = "SHFO";
        keyConstants.bip44CoinType = 133;

        consensus.fCoinbaseMustBeShielded = true;
        consensus.nSubsidySlowStartInterval = 20000;

        // Halving and other parameters

        // --- IMPORTANT: REPLACE FUNDING ADDRESSES TO AVOID CRASHES ---

        // Founders reward script expects a vector of 2-of-3 multisig addresses
        vFoundersRewardAddress = {
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 0*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 1*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 2*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 3*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 4*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 5*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 6*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 7*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 8*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 9*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 10*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 11*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 12*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 13*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 14*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 15*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 16*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 17*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 18*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 19*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 20*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 21*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 22*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 23*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 24*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 25*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 26*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 27*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 28*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 29*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 30*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 31*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 32*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 33*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 34*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 35*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 36*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 37*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 38*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 39*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 40*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 41*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 42*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 43*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 44*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 45*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 46*/
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", /* main-index: 47*/
        };

        // General founder addresses
        std::vector<std::string> founder_addresses(48, "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi");

        // Mining pool addresses
        std::vector<std::string> mining_pool_addresses(48, "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi");

        // FPF addresses for 36 periods
        std::vector<std::string> fpf_addresses_h3(36, "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi");

        // NU6 lockbox address
        std::string nu6_1_kho_address = "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi";

        // The rest of your chainparams.cpp adjusted accordingly...
    }
};
