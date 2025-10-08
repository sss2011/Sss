// Copyright (c) 2020-2023 The Zcash developers
// Copyright (c) 2025 The Shifocoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include <consensus/funding.h>

namespace Consensus
{
    // SHIFOCOIN CUSTOM DISTRIBUTION
    // Founder: 14.59% of block subsidy
    // Mining Pool: 7% of block subsidy
    // Miners: 78.41% automatically (remainder)
    //
    // The valueNumerator and valueDenominator are set in chainparams.cpp
    // for each funding stream (FS_ZIP214_BP for Founder, FS_ZIP214_ZF for Mining Pool)
    
    CAmount FSInfo::Value(CAmount blockSubsidy) const
    {
        // Integer division is floor division for nonnegative integers in C++
        // This calculates: (blockSubsidy * numerator) / denominator
        // Example: For 14.59% = (blockSubsidy * 1459) / 10000
        return CAmount((blockSubsidy * valueNumerator) / valueDenominator);
    }
} // namespace Consensus
