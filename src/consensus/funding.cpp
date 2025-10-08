// Copyright (c) 2020-2023 The Zcash developers
// Copyright (c) 2025 The Shifocoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include <consensus/funding.h>
#include <consensus/params.h>

namespace Consensus
{
    // SHIFOCOIN FUNDING STREAM DISTRIBUTION TABLE
    // This array defines the percentage each funding stream receives
    // Index corresponds to FundingStreamIndex enum in params.h
    //
    // Distribution:
    // - Founder (FS_ZIP214_BP): 14.59% of block subsidy
    // - Mining Pool (FS_ZIP214_ZF): 7% of block subsidy  
    // - All others: 0% (disabled)
    // - Miners: 78.41% automatically (100% - 14.59% - 7%)
    
    const FSInfo FundingStreamInfo[MAX_FUNDING_STREAMS] = {
        // [0] FS_ZIP214_BP - SHIFOCOIN FOUNDER: 14.59%
        {
            .recipient = "Shifocoin Founder",
            .specification = "Shifocoin Distribution Model",
            .valueNumerator = 1459,
            .valueDenominator = 10000
        },
        
        // [1] FS_ZIP214_ZF - SHIFOCOIN MINING POOL: 7%
        {
            .recipient = "Shifocoin Mining Pool",
            .specification = "Shifocoin Distribution Model",
            .valueNumerator = 700,
            .valueDenominator = 10000
        },
        
        // [2] FS_ZIP214_MG - DISABLED (originally Major Grants)
        {
            .recipient = "Disabled",
            .specification = "Not used by Shifocoin",
            .valueNumerator = 0,
            .valueDenominator = 10000
        },
        
        // [3] FS_FPF_ZCG - DISABLED
        {
            .recipient = "Disabled",
            .specification = "Not used by Shifocoin",
            .valueNumerator = 0,
            .valueDenominator = 10000
        },
        
        // [4] FS_DEFERRED - DISABLED
        {
            .recipient = "Disabled",
            .specification = "Not used by Shifocoin",
            .valueNumerator = 0,
            .valueDenominator = 10000
        },
        
        // [5] FS_FPF_ZCG_H3 - DISABLED
        {
            .recipient = "Disabled",
            .specification = "Not used by Shifocoin",
            .valueNumerator = 0,
            .valueDenominator = 10000
        },
        
        // [6] FS_CCF_H3 - DISABLED
        {
            .recipient = "Disabled",
            .specification = "Not used by Shifocoin",
            .valueNumerator = 0,
            .valueDenominator = 10000
        }
    };

    // Calculate the funding stream value based on block subsidy
    CAmount FSInfo::Value(CAmount blockSubsidy) const
    {
        // Integer division is floor division for nonnegative integers in C++
        // Formula: (blockSubsidy × valueNumerator) ÷ valueDenominator
        //
        // Examples with 12.5 SHFO block reward:
        // - Founder:     (12.5 × 1459) / 10000 = 1.82375 SHFO (14.59%)
        // - Mining Pool: (12.5 × 700)  / 10000 = 0.875 SHFO   (7%)
        // - Miner:       12.5 - 1.82375 - 0.875 = 9.80125 SHFO (78.41%)
        
        return CAmount((blockSubsidy * valueNumerator) / valueDenominator);
    }
    
} // namespace Consensus
