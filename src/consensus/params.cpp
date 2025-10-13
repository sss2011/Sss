// Copyright (c) 2019-2023 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "params.h"

#include <amount.h>
#include <key_io.h>
#include <script/standard.h>
#include "upgrades.h"
#include "util/system.h"
#include "util/match.h"

namespace Consensus {
    /**
     * General information about each funding stream.
     * Ordered by Consensus::FundingStreamIndex.
     * NOTE: FundingStreamInfo array is now defined in funding.cpp
     */
    
    static constexpr bool validateFundingStreamInfo(uint32_t idx) {
        return (idx >= Consensus::MAX_FUNDING_STREAMS || (
            FundingStreamInfo[idx].valueNumerator < FundingStreamInfo[idx].valueDenominator &&
            FundingStreamInfo[idx].valueNumerator < (INT64_MAX / MAX_MONEY) &&
            validateFundingStreamInfo(idx + 1)));
    }
    static_assert(
        validateFundingStreamInfo(Consensus::FIRST_FUNDING_STREAM),
        "Invalid FundingStreamInfo");

    std::optional<int> Params::GetActivationHeight(Consensus::UpgradeIndex idx) const {
        auto nActivationHeight = vUpgrades[idx].nActivationHeight;
        if (nActivationHeight == Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT) {
            return std::nullopt;
        } else {
            return nActivationHeight;
        }
    }

    bool Params::NetworkUpgradeActive(int nHeight, Consensus::UpgradeIndex idx) const {
        return NetworkUpgradeState(nHeight, *this, idx) == UPGRADE_ACTIVE;
    }

    int Params::HeightOfLatestSettledUpgrade() const {
        for (auto idxInt = Consensus::MAX_NETWORK_UPGRADES - 1; idxInt > Consensus::BASE_SPROUT; idxInt--) {
            if (vUpgrades[idxInt].hashActivationBlock.has_value()) {
                return vUpgrades[idxInt].nActivationHeight;
            }
        }
        return 0;
    }

    bool Params::FeatureRequired(const Consensus::ConsensusFeature feature) const {
        return vRequiredFeatures.count(feature) > 0;
    }

    bool Params::FeatureActive(const int nHeight, const Consensus::ConsensusFeature feature) const {
        return Features.FeatureActive(*this, nHeight, feature);
    }

    bool Params::FutureTimestampSoftForkActive(int nHeight) const {
        return nHeight >= nFutureTimestampSoftForkHeight;
    }

    int Params::Halving(int nHeight) const {
        // zip208
        // Halving(height) :=
        // floor((height - SlowStartShift) / PreBlossomHalvingInterval), if not IsBlossomActivated(height)
        // floor((BlossomActivationHeight - SlowStartShift) / PreBlossomHalvingInterval + (height - BlossomActivationHeight) / PostBlossomHalvingInterval), otherwise
        if (NetworkUpgradeActive(nHeight, Consensus::UPGRADE_BLOSSOM)) {
            int64_t blossomActivationHeight = vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight;
            // Ideally we would say:
            // halvings = (blossomActivationHeight - SubsidySlowStartShift()) / nPreBlossomSubsidyHalvingInterval
            //     + (nHeight - blossomActivationHeight) / nPostBlossomSubsidyHalvingInterval;
            // But, (blossomActivationHeight - SubsidySlowStartShift()) / nPreBlossomSubsidyHalvingInterval
            // would need to be treated as a rational number in order for this to work.
            // Define scaledHalvings := halvings * nPostBlossomSubsidyHalvingInterval;
            int64_t scaledHalvings = ((blossomActivationHeight - SubsidySlowStartShift()) * Consensus::BLOSSOM_POW_TARGET_SPACING_RATIO)
                + (nHeight - blossomActivationHeight);
            return (int) (scaledHalvings / nPostBlossomSubsidyHalvingInterval);
        } else {
            return (nHeight - SubsidySlowStartShift()) / nPreBlossomSubsidyHalvingInterval;
        }
    }

    /**
     * This method determines the block height of the `halvingIndex`th
     * halving, as known at the specified `nHeight` block height.
     *
     * Previous implementations of this logic were specialized to the
     * first halving.
     */
    int Params::HalvingHeight(int nHeight, int halvingIndex) const {
        assert(nHeight >= 0);
        assert(halvingIndex > 0);

        // zip208
        // HalvingHeight(i) := max({ height ⦂ N | Halving(height) < i }) + 1
        //
        // Halving(h) returns the halving index at the specified height.  It is
        // defined as floor(f(h)) where f is a strictly increasing rational
        // function, so it's sufficient to solve for f(height) = halvingIndex
        // in the rationals and then take ceiling(height).
        //
        // H := blossom activation height;
        // SS := SubsidySlowStartShift();
        // R := 1 / (postInterval / preInterval) = BLOSSOM_POW_TARGET_SPACING_RATIO
        // (The following calculation depends on BLOSSOM_POW_TARGET_SPACING_RATIO being an integer.)
        //
        // preBlossom:
        // i = (height - SS) / preInterval
        // height = (preInterval * i) + SS
        //
        // postBlossom:
        // i = (H - SS) / preInterval + (HalvingHeight(i) - H) / postInterval
        // preInterval = postInterval / R
        // i = (H - SS) / (postInterval / R) + (HalvingHeight(i) - H) / postInterval
        // i = (R * (H - SS) + HalvingHeight(i) - H) / postInterval
        // postInterval * i = R * (H - SS) + HalvingHeight(i) - H
        // HalvingHeight(i) = postInterval * i - R * (H - SS) + H
        if (NetworkUpgradeActive(nHeight, Consensus::UPGRADE_BLOSSOM)) {
            int blossomActivationHeight = vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight;

            return
                (nPostBlossomSubsidyHalvingInterval * halvingIndex)
                - (BLOSSOM_POW_TARGET_SPACING_RATIO * (blossomActivationHeight - SubsidySlowStartShift()))
                + blossomActivationHeight;
        } else {
            return (nPreBlossomSubsidyHalvingInterval * halvingIndex) + SubsidySlowStartShift();
        }
    }

    int Params::GetLastFoundersRewardBlockHeight(int nHeight) const {
        return HalvingHeight(nHeight, 1) - 1;
    }

    int Params::FundingPeriodIndex(int fundingStreamStartHeight, int nHeight) const {
        assert(fundingStreamStartHeight <= nHeight);

        int firstHalvingHeight = HalvingHeight(fundingStreamStartHeight, 1);

        // If the start height of the funding period is not aligned to a multiple of the
        // funding period length, the first funding period will be shorter than the
        // funding period length.
        auto startPeriodOffset = (fundingStreamStartHeight - firstHalvingHeight) % nFundingPeriodLength;
        if (startPeriodOffset < 0) startPeriodOffset += nFundingPeriodLength; // C++ '%' is remainder, not modulus!

        return (nHeight - fundingStreamStartHeight + startPeriodOffset) / nFundingPeriodLength;
    }

    std::variant<FundingStream, FundingStreamError> FundingStream::ValidateFundingStream(
        const Consensus::Params& params,
        const int startHeight,
        const int endHeight,
        const std::vector<FundingStreamRecipient>& recipients
    ) {
        if (!params.NetworkUpgradeActive(startHeight, Consensus::UPGRADE_CANOPY)) {
            return FundingStreamError::CANOPY_NOT_ACTIVE;
        }

        if (endHeight < startHeight) {
            return FundingStreamError::ILLEGAL_RANGE;
        }

        const auto expectedRecipients = params.FundingPeriodIndex(startHeight, endHeight - 1) + 1;
        if (expectedRecipients > recipients.size()) {
            return FundingStreamError::INSUFFICIENT_RECIPIENTS;
        }

        // Lockbox output periods must not start before NU6
        if (!params.NetworkUpgradeActive(startHeight, Consensus::UPGRADE_NU6)) {
            for (auto recipient : recipients) {
                if (std::holds_alternative<Consensus::Lockbox>(recipient)) {
                    return FundingStreamError::NU6_NOT_ACTIVE;
                }
            }
        }

        return FundingStream(startHeight, endHeight, recipients);
    };

    class GetFundingStreamOrThrow {
    public:
        FundingStream operator()(const FundingStream& fs) const {
            return fs;
        }

        FundingStream operator()(const FundingStreamError& e) const {
            switch (e) {
                case FundingStreamError::CANOPY_NOT_ACTIVE:
                    throw std::runtime_error("Canopy network upgrade not active at funding stream start height.");
                case FundingStreamError::ILLEGAL_RANGE:
                    throw std::runtime_error("Illegal start/end height combination for funding stream.");
                case FundingStreamError::INSUFFICIENT_RECIPIENTS:
                    throw std::runtime_error("Insufficient recipient identifiers to fully exhaust funding stream.");
                case FundingStreamError::NU6_NOT_ACTIVE:
                    throw std::runtime_error("NU6 network upgrade not active at lockbox period start height.");
                default:
                    throw std::runtime_error("Unrecognized error validating funding stream.");
            };
        }
    };

    FundingStream FundingStream::ParseFundingStream(
        const Consensus::Params& params,
        const KeyConstants& keyConstants,
        const int startHeight,
        const int endHeight,
        const std::vector<std::string>& strAddresses,
        const bool allowDeferredPool)
    {
        KeyIO keyIO(keyConstants);

        // Parse the address strings into concrete types.
        std::vector<FundingStreamRecipient> recipients;
        for (const auto& strAddr : strAddresses) {
            if (allowDeferredPool && strAddr == "DEFERRED_POOL") {
                recipients.push_back(Lockbox());
                continue;
            }

            auto addr = keyIO.DecodePaymentAddress(strAddr);
            if (!addr.has_value()) {
                throw std::runtime_error("Funding stream address was not a valid " PACKAGE_NAME " address.");
            }

            examine(addr.value(), match {
                [&](const CKeyID& keyId) {
                    recipients.push_back(GetScriptForDestination(keyId));
                },
                [&](const CScriptID& scriptId) {
                    recipients.push_back(GetScriptForDestination(scriptId));
                },
                [&](const libzcash::SaplingPaymentAddress& zaddr) {
                    recipients.push_back(zaddr);
                },
                [&](const auto& zaddr) {
                    throw std::runtime_error("Funding stream address was not a valid transparent P2SH or Sapling address.");
                }
            });
        }

        auto validationResult = FundingStream::ValidateFundingStream(params, startHeight, endHeight, recipients);
        return std::visit(GetFundingStreamOrThrow(), validationResult);
    };

    OnetimeLockboxDisbursement OnetimeLockboxDisbursement::Parse(
        const Consensus::Params& params,
        const KeyConstants& keyConstants,
        const UpgradeIndex upgrade,
        const CAmount zatoshis,
        const std::string& strAddress)
    {
        KeyIO keyIO(keyConstants);

        if (upgrade < Consensus::UPGRADE_NU6_1) {
            throw std::runtime_error("Cannot define one-time lockbox disbursements prior to NU6.1.");
        }

        // Parse the address string into concrete types.
        auto addr = keyIO.DecodePaymentAddress(strAddress);
        if (!addr.has_value()) {
            throw std::runtime_error("One-time lockbox disbursement address was not a valid " PACKAGE_NAME " address.");
        }

        CScript recipient;
        examine(addr.value(), match {
            [&](const CScriptID& scriptId) {
                recipient = GetScriptForDestination(scriptId);
            },
            [&](const auto& zaddr) {
                throw std::runtime_error("One-time lockbox disbursement address was not a valid transparent P2SH address.");
            }
        });

        // TODO: Consider verifying that the set of (recipient, amount) tuples
        // are distinct from all possible funding stream tuples.
        return OnetimeLockboxDisbursement(upgrade, zatoshis, recipient);
    };

    void Params::AddZIP207FundingStream(
        const KeyConstants& keyConstants,
        FundingStreamIndex idx,
        int startHeight,
        int endHeight,
        const std::vector<std::string>& strAddresses)
    {
        vFundingStreams[idx] = FundingStream::ParseFundingStream(
                *this, keyConstants,
                startHeight, endHeight, strAddresses,
                false);
    };

    void Params::AddZIP207LockboxStream(
        const KeyConstants& keyConstants,
        FundingStreamIndex idx,
        int startHeight,
        int endHeight)
    {
        auto intervalCount = FundingPeriodIndex(startHeight, endHeight - 1) + 1;
        std::vector<FundingStreamRecipient> recipients(intervalCount, Lockbox());
        auto validationResult = FundingStream::ValidateFundingStream(
                *this,
                startHeight,
                endHeight,
                recipients);
        vFundingStreams[idx] = std::visit(GetFundingStreamOrThrow(), validationResult);
    };

    void Params::AddZIP271LockboxDisbursement(
        const KeyConstants& keyConstants,
        OnetimeLockboxDisbursementIndex idx,
        UpgradeIndex upgrade,
        CAmount zatoshis,
        const std::string& strAddress)
    {
        vOnetimeLockboxDisbursements[idx] = OnetimeLockboxDisbursement::Parse(
                *this, keyConstants,
                upgrade, zatoshis, strAddress);
    };

    CAmount Params::GetBlockSubsidy(int nHeight) const
    {
        CAmount nSubsidy = 12.5 * COIN;

        // Mining slow start
        // The subsidy is ramped up linearly, skipping the middle payout of
        // MAX_SUBSIDY/2 to keep the monetary curve consistent with no slow start.
        if (nHeight < this->SubsidySlowStartShift()) {
            nSubsidy /= this->nSubsidySlowStartInterval;
            nSubsidy *= nHeight;
            return nSubsidy;
        } else if (nHeight < this->nSubsidySlowStartInterval) {
            nSubsidy /= this->nSubsidySlowStartInterval;
            nSubsidy *= (nHeight+1);
            return nSubsidy;
        }

        assert(nHeight >= this->SubsidySlowStartShift());

        int halvings = this->Halving(nHeight);

        // Force block reward to zero when right shift is undefined.
        if (halvings >= 64)
            return 0;

        // zip208
        // BlockSubsidy(height) :=
        // SlowStartRate · height, if height < SlowStartInterval / 2
        // SlowStartRate · (height + 1), if SlowStartInterval / 2 ≤ height and height < SlowStartInterval
        // floor(MaxBlockSubsidy / 2^Halving(height)), if SlowStartInterval ≤ height and not IsBlossomActivated(height)
        // floor(MaxBlockSubsidy / (BlossomPoWTargetSpacingRatio · 2^Halving(height))), otherwise
        if (this->NetworkUpgradeActive(nHeight, Consensus::UPGRADE_BLOSSOM)) {
            return (nSubsidy / Consensus::BLOSSOM_POW_TARGET_SPACING_RATIO) >> halvings;
        } else {
            // Subsidy is cut in half every 840,000 blocks which will occur approximately every 4 years.
            return nSubsidy >> halvings;
        }
    }

    std::vector<std::pair<FSInfo, FundingStream>> Params::GetActiveFundingStreams(int nHeight) const
    {
        std::vector<std::pair<FSInfo, FundingStream>> activeStreams;

        // Funding streams are disabled if Canopy is not active.
        if (NetworkUpgradeActive(nHeight, Consensus::UPGRADE_CANOPY)) {
            for (uint32_t idx = Consensus::FIRST_FUNDING_STREAM; idx < Consensus::MAX_FUNDING_STREAMS; idx++) {
                // The following indexed access is safe as Consensus::MAX_FUNDING_STREAMS is used
                // in the definition of vFundingStreams.
                auto fs = vFundingStreams[idx];

                // Funding period is [startHeight, endHeight).
                if (fs && nHeight >= fs.value().GetStartHeight() && nHeight < fs.value().GetEndHeight()) {
                    activeStreams.push_back(std::make_pair(FundingStreamInfo[idx], fs.value()));
                }
            }
        }

        return activeStreams;
    };

    std::set<FundingStreamElement> Params::GetActiveFundingStreamElements(int nHeight) const
    {
        return GetActiveFundingStreamElements(nHeight, GetBlockSubsidy(nHeight));
    }

    std::set<FundingStreamElement> Params::GetActiveFundingStreamElements(
        int nHeight,
        CAmount blockSubsidy) const
    {
        std::set<std::pair<FundingStreamRecipient, CAmount>> requiredElements;

        // Funding streams are disabled if Canopy is not active.
        if (NetworkUpgradeActive(nHeight, Consensus::UPGRADE_CANOPY)) {
            for (const auto& [fsinfo, fs] : GetActiveFundingStreams(nHeight)) {
                requiredElements.insert(std::make_pair(
                    fs.Recipient(*this, nHeight),
                    fsinfo.Value(blockSubsidy)));
            }
        }

        return requiredElements;
    };

    std::vector<OnetimeLockboxDisbursement> Params::GetLockboxDisbursementsForHeight(int nHeight) const
    {
        std::vector<OnetimeLockboxDisbursement> disbursements;

        // Disbursements are disabled if NU6.1 is not active.
        if (NetworkUpgradeActive(nHeight, Consensus::UPGRADE_NU6_1)) {
            for (uint32_t idx = Consensus::FIRST_ONETIME_LOCKBOX_DISBURSEMENT; idx < Consensus::MAX_ONETIME_LOCKBOX_DISBURSEMENTS; idx++) {
                // The following indexed access is safe as
                // Consensus::MAX_ONETIME_LOCKBOX_DISBURSEMENTS is used
                // in the definition of vOnetimeLockboxDisbursements.
                auto ld = vOnetimeLockboxDisbursements[idx];

                if (ld && GetActivationHeight(ld.value().GetUpgrade()) == nHeight) {
                    disbursements.push_back(ld.value());
                }
            }
        }

        return disbursements;
    };

    FundingStreamRecipient FundingStream::Recipient(const Consensus::Params& params, int nHeight) const
    {
        auto addressIndex = params.FundingPeriodIndex(startHeight, nHeight);

        assert(addressIndex >= 0 && addressIndex < recipients.size());
        return recipients[addressIndex];
    };

    int64_t Params::PoWTargetSpacing(int nHeight) const {
        // zip208
        // PoWTargetSpacing(height) :=
        // PreBlossomPoWTargetSpacing, if not IsBlossomActivated(height)
        // PostBlossomPoWTargetSpacing, otherwise.
        bool blossomActive = NetworkUpgradeActive(nHeight, Consensus::UPGRADE_BLOSSOM);
        return blossomActive ? nPostBlossomPowTargetSpacing : nPreBlossomPowTargetSpacing;
    }

    int64_t Params::AveragingWindowTimespan(int nHeight) const {
        return nPowAveragingWindow * PoWTargetSpacing(nHeight);
    }

    int64_t Params::MinActualTimespan(int nHeight) const {
        return (AveragingWindowTimespan(nHeight) * (100 - nPowMaxAdjustUp)) / 100;
    }

    int64_t Params::MaxActualTimespan(int nHeight) const {
        return (AveragingWindowTimespan(nHeight) * (100 + nPowMaxAdjustDown)) / 100;
    }
}
