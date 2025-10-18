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

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    // To create a genesis block for a new chain which is Overwintered:
    //   txNew.nVersion = OVERWINTER_TX_VERSION
    //   txNew.fOverwintered = true
    //   txNew.nVersionGroupId = OVERWINTER_VERSION_GROUP_ID
    //   txNew.nExpiryHeight = <default value>
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 520617983 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nSolution = nSolution;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database (and is in any case of zero value).
 *
 * >>> from hashlib import blake2s
 * >>> 'Zcash' + blake2s(b'The Economist 2016-10-29 Known unknown: Another crypto-currency is born. BTC#436254 0000000000000000044f321997f336d2908cf8c8d6893e88dbf067e2d949487d ETH#2521903 483039a6b6bd8bd05f0584f9a078d075e454925eb71c1f13eaff59b405a721bb DJIA close on 27 Oct 2016: 18,169.68').hexdigest()
 *
 * CBlock(hash=00040fe8, ver=4, hashPrevBlock=00000000000000, hashMerkleRoot=c4eaa5, nTime=1477641360, nBits=1f07ffff, nNonce=4695, vtx=1)
 *   CTransaction(hash=c4eaa5, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff071f0104455a6361736830623963346565663862376363343137656535303031653335303039383462366665613335363833613763616331343161303433633432303634383335643334)
 *     CTxOut(nValue=0.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: c4eaa5
 */
static CBlock CreateGenesisBlock(uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Shifocoin - Privacy by choice freedom by design - October 2025";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nSolution, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

const arith_uint256 maxUint = UintToArith256(uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));

class CMainParams : public CChainParams {
public:
    CMainParams() {
        keyConstants.strNetworkID = "shifocoin";
        strCurrencyUnits = "SHFO";    // Standard cryptocurrency ticker format
        keyConstants.bip44CoinType = 133; // As registered in https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        consensus.fCoinbaseMustBeShielded = true;
        consensus.nSubsidySlowStartInterval = 20000;
        consensus.nPreBlossomSubsidyHalvingInterval = 5760000;  // Adjusted for 144M total supply (6.857x Zcash)
        consensus.nPostBlossomSubsidyHalvingInterval = POST_BLOSSOM_HALVING_INTERVAL(Consensus::PRE_BLOSSOM_HALVING_INTERVAL);
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 4000;
        const size_t N = 200, K = 9;
        static_assert(equihash_parameters_acceptable(N, K));
        consensus.nEquihashN = N;
        consensus.nEquihashK = K;
        consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = std::nullopt;
        consensus.fPowNoRetargeting = false;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170005;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight = 347500;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].hashActivationBlock =
            uint256S("0000000003761c0d0c3974b54bdb425613bbb1eaadd6e70b764de82f195ea243");
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170007;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 419200;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].hashActivationBlock =
            uint256S("00000000025a57200d898ac7f21e26bf29028bbe96ec46e05b2c17cc9db9e4f3");
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170009;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight = 653600;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].hashActivationBlock =
            uint256S("00000000020bebb33c1b34b67a982a328ab212a206dacbe561a7cc94aab3e9bb");
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nProtocolVersion = 170011;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nActivationHeight = 903000;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].hashActivationBlock =
            uint256S("0000000000aad1c8698964a93c35ecf8b4d05e848de9e2fe7606067139be5643");
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nProtocolVersion = 170013;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight = 1046400;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].hashActivationBlock =
            uint256S("00000000002038016f976744c369dce7419fca30e7171dfac703af5e5f7ad1d4");
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nProtocolVersion = 170100;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nActivationHeight = 1687104;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].hashActivationBlock =
            uint256S("0000000000d723156d9b65ffcf4984da7a19675ed7e2f06d9e5d5188af087bf8");
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nProtocolVersion = 170120;
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nActivationHeight = 2726400;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nProtocolVersion = 170140;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nActivationHeight = 3146400;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nProtocolVersion = 0x7FFFFFFF;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.nFundingPeriodLength = consensus.nPostBlossomSubsidyHalvingInterval / 48;
// SHIFOCOIN TEMPORARY FIX: Use testnet prefixes to match t2 addresses
// guarantees the first 2 characters are "tm" (testnet mainnet)
keyConstants.base58Prefixes[PUBKEY_ADDRESS]     = {0x1D,0x25};
// guarantees the first 2 characters are "t2" (testnet script)
keyConstants.base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0xBA};
     // the first character, when base58 encoded, is "5" or "K" or "L" (as in Bitcoin)
        keyConstants.base58Prefixes[SECRET_KEY]         = {0x80};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x88,0xB2,0x1E};
        keyConstants.base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x88,0xAD,0xE4};
        // guarantees the first 2 characters, when base58 encoded, are "zc"
        keyConstants.base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0x9A};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVK"
        keyConstants.base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAB,0xD3};
        // guarantees the first 2 characters, when base58 encoded, are "SK"
        keyConstants.base58Prefixes[ZCSPENDING_KEY]     = {0xAB,0x36};

        keyConstants.bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "zs";
        keyConstants.bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviews";
        keyConstants.bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivks";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-main";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_FVK]         = "zxviews";

        keyConstants.bech32mHRPs[TEX_ADDRESS]                 = "tex";
        {
            auto canopyActivation = consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight;
            auto nu6Activation = consensus.vUpgrades[Consensus::UPGRADE_NU6].nActivationHeight;
            auto nu6_1Activation = consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nActivationHeight;
        // SHIFOCOIN CUSTOM DISTRIBUTION
// 14.59% to Founder + 7% to Mining Pool + 78.41% to Miners
// Using temporary valid addresses - replace with real Shifocoin addresses after genesis mining
std::vector<std::string> founder_addresses(48, "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi");
std::vector<std::string> mining_pool_addresses(48, "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi");

        // Add founder reward stream (14.59%)
        consensus.AddZIP207FundingStream(
            keyConstants,
            Consensus::FS_ZIP214_BP,  // Reusing enum for founder rewards
            canopyActivation,
            nu6Activation,
            founder_addresses);

        // Add mining pool stream (7%)
        consensus.AddZIP207FundingStream(
            keyConstants,
            Consensus::FS_ZIP214_ZF,  // Reusing enum for mining pool
            canopyActivation,
            nu6Activation,
            mining_pool_addresses);

        // Remove the MG funding stream entirely (we only use 2 streams)
        // Remaining 78.41% automatically goes to block miners


            // ZIP 214 Revision 1
            // FPF uses a single address repeated 12 times, once for each funding period.
            std::vector<std::string> fpf_addresses(12, "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi");

            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_FPF_ZCG,
                nu6Activation,
                nu6_1Activation,
                fpf_addresses);
            consensus.AddZIP207LockboxStream(
                keyConstants,
                Consensus::FS_DEFERRED,
                nu6Activation,
                nu6_1Activation);

            // ZIP 214 Revision 2
            // FPF uses a single address repeated 36 times, once for each funding period.
            std::vector<std::string> fpf_addresses_h3(36, "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi");
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_FPF_ZCG_H3,
                nu6_1Activation,
                4406400,
                fpf_addresses_h3);
            consensus.AddZIP207LockboxStream(
                keyConstants,
                Consensus::FS_CCF_H3,
                nu6_1Activation,
                4406400);

            // ZIP 271
            // For convenience of distribution, we split the lockbox contents into 10 equal chunks.
            std::string nu6_1_kho_address = "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi";
            static const CAmount nu6_1_disbursement_amount = 78750 * COIN;
            static const CAmount nu6_1_chunk_amount = 7875 * COIN;
            static constexpr auto nu6_1_chunks = {
                Consensus::LD_ZIP271_NU6_1_CHUNK_1,
                Consensus::LD_ZIP271_NU6_1_CHUNK_2,
                Consensus::LD_ZIP271_NU6_1_CHUNK_3,
                Consensus::LD_ZIP271_NU6_1_CHUNK_4,
                Consensus::LD_ZIP271_NU6_1_CHUNK_5,
                Consensus::LD_ZIP271_NU6_1_CHUNK_6,
                Consensus::LD_ZIP271_NU6_1_CHUNK_7,
                Consensus::LD_ZIP271_NU6_1_CHUNK_8,
                Consensus::LD_ZIP271_NU6_1_CHUNK_9,
                Consensus::LD_ZIP271_NU6_1_CHUNK_10,
            };
            static_assert(nu6_1_chunk_amount * nu6_1_chunks.size() == nu6_1_disbursement_amount);
            for (auto idx : nu6_1_chunks) {
                consensus.AddZIP271LockboxDisbursement(
                    keyConstants,
                    idx,
                    Consensus::UPGRADE_NU6_1,
                    nu6_1_chunk_amount,
                    nu6_1_kho_address);
            }
        }

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000001517f0d837f57259");

        /**
         * The message start string should be awesome! ⓩ❤
         */
        pchMessageStart[0] = 0x53;    // Change to 0x53 for 'S'
        pchMessageStart[1] = 0x48;  // 'H'
        pchMessageStart[2] = 0x46;  // 'F'
        pchMessageStart[3] = 0x4F;  // 'O'
        vAlertPubKey = ParseHex("04b7ecf0baa90495ceb4e4090f6b2fd37eec1e9c85fac68a487f3ce11589692e4a317479316ee814e066638e1db54e37a10689b70286e6315b1087b6615d179264");
        nDefaultPort = 9033;    // Shifocoin port
        nPruneAfterHeight = 100000;

genesis = CreateGenesisBlock(
    1728163200,  // Unix timestamp for October 2025
    uint256S("0x0000000000000000000000000000000000000000000000000000000000000000"),  
    ParseHex("00"),  
    0x1f07ffff, 4, 0);

// ===== TEMPORARY GENESIS MINING CODE =====
if (true) {  // Always mine on first run
    printf("Mining mainnet genesis block...\n");
    arith_uint256 hashTarget = arith_uint256().SetCompact(genesis.nBits);
    
    while (UintToArith256(genesis.GetHash()) > hashTarget) {
        genesis.nNonce = ArithToUint256(UintToArith256(genesis.nNonce) + 1);
        
       if (UintToArith256(genesis.nNonce).GetUint64(0) % 10000 == 0) {
    printf("Tried %s nonces...\n", UintToArith256(genesis.nNonce).ToString().c_str());
}
    }
    
    printf("\n=== MAINNET GENESIS BLOCK FOUND ===\n");
    printf("nTime: %u\n", genesis.nTime);
    printf("nBits: 0x%08x\n", genesis.nBits);
    printf("nNonce: %s\n", genesis.nNonce.ToString().c_str());
    printf("GetHash: %s\n", genesis.GetHash().ToString().c_str());
    printf("hashMerkleRoot: %s\n", genesis.hashMerkleRoot.ToString().c_str());
    printf("nSolution (hex): ");
    for (size_t i = 0; i < genesis.nSolution.size(); i++) {
        printf("%02x", genesis.nSolution[i]);
    }
    printf("\n===================================\n\n");
    exit(0);  // Exit after finding genesis
}
// ===== END MINING CODE =====

consensus.hashGenesisBlock = genesis.GetHash();

consensus.hashGenesisBlock = genesis.GetHash();

        vFixedSeeds.clear();
        vSeeds.clear();// Shifocoin seed nodes - ADD YOUR OWN SEED NODES HERE LATER
// vSeeds.push_back(CDNSSeedData("shifocoin.org", "seed1.shifocoin.org"));
// For now, leave empty until you set up seed nodes


        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (      0, consensus.hashGenesisBlock)
            (   2500, uint256S("0x00000006dc968f600be11a86cbfbf7feb61c7577f45caced2e82b6d261d19744"))
            (  15000, uint256S("0x00000000b6bc56656812a5b8dcad69d6ad4446dec23b5ec456c18641fb5381ba"))
            (  67500, uint256S("0x000000006b366d2c1649a6ebb4787ac2b39c422f451880bc922e3a6fbd723616"))
            ( 100000, uint256S("0x000000001c5c82cd6baccfc0879e3830fd50d5ede17fa2c37a9a253c610eb285"))
            ( 133337, uint256S("0x0000000002776ccfaf06cc19857accf3e20c01965282f916b8a886e3e4a05be9"))
            ( 180000, uint256S("0x000000001205b742eac4a1b3959635bdf8aeada078d6a996df89740f7b54351d"))
            ( 222222, uint256S("0x000000000cafb9e56445a6cabc8057b57ee6fcc709e7adbfa195e5c7fac61343"))
            ( 270000, uint256S("0x00000000025c1cfa0258e33ab050aaa9338a3d4aaa3eb41defefc887779a9729"))
            ( 304600, uint256S("0x00000000028324e022a45014c4a4dc51e95d41e6bceb6ad554c5b65d5cea3ea5"))
            ( 410100, uint256S("0x0000000002c565958f783a24a4ac17cde898ff525e75ed9baf66861b0b9fcada"))
            ( 497000, uint256S("0x0000000000abd333f0acca6ffdf78a167699686d6a7d25c33fca5f295061ffff"))
            ( 525000, uint256S("0x0000000001a36c500378be8862d9bf1bea8f1616da6e155971b608139cc7e39b"))
            ( 650000, uint256S("0x0000000000a0a3fbbd739fb4fcbbfefff44efffc2064ca69a59d5284a2da26e2"))
            ( 800000, uint256S("0x00000000013f1f4e5634e896ebdbe63dec115547c1480de0d83c64426f913c27"))
            (1000000, uint256S("0x000000000062eff9ae053020017bfef24e521a2704c5ec9ead2a4608ac70fc7a"))
            (1200000, uint256S("0x0000000000347d5011108fdcf667c93e622e8635c94e586556898e41db18d192"))
            (1400000, uint256S("0x0000000001155ecec0ad3924d47ad476c0a5ed7527b8776f53cbda1a780b9f76"))
            (1600000, uint256S("0x0000000000aae69fb228f90e77f34c24b7920667eaca726c3a3939536f03dcfc"))
            (1860000, uint256S("0x000000000043a968c78af5fb8133e00e6fe340051c19dd969e53ab62bf3dc22a"))
            (2000000, uint256S("0x00000000010accaf2f87934765dc2e0bf4823a2b1ae2c1395b334acfce52ad68"))
            (2200000, uint256S("0x0000000001a0139c4c4d0e8f68cc562227c6003f4b1b640a3d921aeb8c3d2e3d"))
            (2400000, uint256S("0x0000000000294d1c8d87a1b6566d302aa983691bc3cab0583a245389bbb9d285"))
            (2600000, uint256S("0x0000000000b5ad92fcec0069d590f674d05ec7d96b1ff727863ea390950c4e49"))
            (2800000, uint256S("0x00000000011a226fb25d778d65b055605a82da016989b7788e0ce83c4f8d64f7"))
            (3000000, uint256S("0x0000000000573729e4db33678233e5dc0cc721c9c09977c64dcaa3f6344de8e9")),
            1752983473,     // * UNIX timestamp of last checkpoint block
            15537904,       // * total number of transactions between genesis and last checkpoint
            5967            // * estimated number of transactions per day after checkpoint
                            //   (total number of tx * 48 * 24) / checkpoint block height
        };

        // Hardcoded fallback value for the Sprout shielded value pool balance
        // for nodes that have not reindexed since the introduction of monitoring
        // in #2795.
        nSproutValuePoolCheckpointHeight = 520633;
        nSproutValuePoolCheckpointBalance = 22145062442933;
        fZIP209Enabled = true;
        hashSproutValuePoolCheckpointBlock = uint256S("0000000000c7b46b6bc04b4cbf87d8bb08722aebd51232619b214f7273f8460e");

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


        assert(vFoundersRewardAddress.size() <= consensus.GetLastFoundersRewardBlockHeight(0));
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        keyConstants.strNetworkID = "test";
        strCurrencyUnits = "TAZ";
        keyConstants.bip44CoinType = 1;
        consensus.fCoinbaseMustBeShielded = true;
        consensus.nSubsidySlowStartInterval = 20000;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = POST_BLOSSOM_HALVING_INTERVAL(Consensus::PRE_BLOSSOM_HALVING_INTERVAL);
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 400;
        const size_t N = 200, K = 9;
        static_assert(equihash_parameters_acceptable(N, K));
        consensus.nEquihashN = N;
        consensus.nEquihashK = K;
        consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = 299187;
        consensus.fPowNoRetargeting = false;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170003;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight = 207500;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].hashActivationBlock =
            uint256S("0000257c4331b098045023fcfbfa2474681f4564ab483f84e4e1ad078e4acf44");
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170007;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 280000;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].hashActivationBlock =
            uint256S("000420e7fcc3a49d729479fb0b560dd7b8617b178a08e9e389620a9d1dd6361a");
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170008;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight = 584000;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].hashActivationBlock =
            uint256S("00367515ef2e781b8c9358b443b6329572599edd02c59e8af67db9785122f298");
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nProtocolVersion = 170010;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nActivationHeight = 903800;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].hashActivationBlock =
            uint256S("05688d8a0e9ff7c04f6f05e6d695dc5ab43b9c4803342d77ae360b2b27d2468e");
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nProtocolVersion = 170012;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight = 1028500;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].hashActivationBlock =
            uint256S("01a4d7c6aada30c87762c1bf33fff5df7266b1fd7616bfdb5227fa59bd79e7a2");
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nProtocolVersion = 170050;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nActivationHeight = 1842420;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].hashActivationBlock =
            uint256S("0006d75c60b3093d1b671ff7da11c99ea535df9927c02e6ed9eb898605eb7381");
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nProtocolVersion = 170110;
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nActivationHeight = 2976000;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nProtocolVersion = 170130;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nActivationHeight = 3536500;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nProtocolVersion = 0x7FFFFFFF;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.nFundingPeriodLength = consensus.nPostBlossomSubsidyHalvingInterval / 48;

        // guarantees the first 2 characters, when base58 encoded, are "tm"
        keyConstants.base58Prefixes[PUBKEY_ADDRESS]     = {0x1D,0x25};
        // guarantees the first 2 characters, when base58 encoded, are "t2"
        keyConstants.base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0xBA};
        // the first character, when base58 encoded, is "9" or "c" (as in Bitcoin)
        keyConstants.base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        keyConstants.base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        // guarantees the first 2 characters, when base58 encoded, are "zt"
        keyConstants.base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0xB6};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVt"
        keyConstants.base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        // guarantees the first 2 characters, when base58 encoded, are "ST"
        keyConstants.base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

        keyConstants.bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "ztestsapling";
        keyConstants.bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviewtestsapling";
        keyConstants.bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivktestsapling";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-test";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_FVK]         = "zxviewtestsapling";

        keyConstants.bech32mHRPs[TEX_ADDRESS]                 = "textest";

        // Testnet funding streams
        {
            auto canopyActivation = consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight;
            auto nu6Activation = consensus.vUpgrades[Consensus::UPGRADE_NU6].nActivationHeight;
            auto nu6_1Activation = consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nActivationHeight;

            // ZIP 214 Revision 0
            std::vector<std::string> bp_addresses = {
                "t26ovBdKAJLtrvBsE2QGF4nqBkEuptuPFZz",
                "t26ovBdKAJLtrvBsE2QGF4nqBkEuptuPFZz",
                "t26ovBdKAJLtrvBsE2QGF4nqBkEuptuPFZz",
                "t26ovBdKAJLtrvBsE2QGF4nqBkEuptuPFZz",
                "t2NNHrgPpE388atmWSF4DxAb3xAoW5Yp45M",
                "t2VMN28itPyMeMHBEd9Z1hm6YLkQcGA1Wwe",
                "t2CHa1TtdfUV8UYhNm7oxbzRyfr8616BYh2",
                "t2F77xtr28U96Z2bC53ZEdTnQSUAyDuoa67",
                "t2ARrzhbgcpoVBDPivUuj6PzXzDkTBPqfcT",
                "t278aQ8XbvFR15mecRguiJDQQVRNnkU8kJw",
                "t2Dp1BGnZsrTXZoEWLyjHmg3EPvmwBnPDGB",
                "t2KzeqXgf4ju33hiSqCuKDb8iHjPCjMq9iL",
                "t2Nyxqv1BiWY1eUSiuxVw36oveawYuo18tr",
                "t2DKFk5JRsVoiuinK8Ti6eM4Yp7v8BbfTyH",
                "t2CUaBca4k1x36SC4q8Nc8eBoqkMpF3CaLg",
                "t296SiKL7L5wvFmEdMxVLz1oYgd6fTfcbZj",
                "t29fBCFbhgsjL3XYEZ1yk1TUh7eTusB6dPg",
                "t2FGofLJXa419A76Gpf5ncxQB4gQXiQMXjK",
                "t2ExfrnRVnRiXDvxerQ8nZbcUQvNvAJA6Qu",
                "t28JUffLp47eKPRHKvwSPzX27i9ow8LSXHx",
                "t2JXWPtrtyL861rFWMZVtm3yfgxAf4H7uPA",
                "t2QdgbJoWfYHgyvEDEZBjHmgkr9yNJff3Hi",
                "t2QW43nkco8r32ZGRN6iw6eSzyDjkMwCV3n",
                "t2DgYDXMJTYLwNcxighQ9RCgPxMVATRcUdC",
                "t2Bop7dg33HGZx3wunnQzi2R2ntfpjuti3M",
                "t2HVeEwovcLq9RstAbYkqngXNEsCe2vjJh9",
                "t2HxbP5keQSx7p592zWQ5bJ5GrMmGDsV2Xa",
                "t2TJzUg2matao3mztBRJoWnJY6ekUau6tPD",
                "t29pMzxmo6wod25YhswcjKv3AFRNiBZHuhj",
                "t2QBQMRiJKYjshJpE6RhbF7GLo51yE6d4wZ",
                "t2F5RqnqguzZeiLtYHFx4yYfy6pDnut7tw5",
                "t2CHvyZANE7XCtg8AhZnrcHCC7Ys1jJhK13",
                "t2BRzpMdrGWZJ2upsaNQv6fSbkbTy7EitLo",
                "t2BFixHGQMAWDY67LyTN514xRAB94iEjXp3",
                "t2Uvz1iVPzBEWfQBH1p7NZJsFhD74tKaG8V",
                "t2CmFDj5q6rJSRZeHf1SdrowinyMNcj438n",
                "t2ErNvWEReTfPDBaNizjMPVssz66aVZh1hZ",
                "t2GeJQ8wBUiHKDVzVM5ZtKfY5reCg7CnASs",
                "t2L2eFtkKv1G6j55kLytKXTGuir4raAy3yr",
                "t2EK2b87dpPazb7VvmEGc8iR6SJ289RywGL",
                "t2DJ7RKeZJxdA4nZn8hRGXE8NUyTzjujph9",
                "t2K1pXo4eByuWpKLkssyMLe8QKUbxnfFC3H",
                "t2TB4mbSpuAcCWkH94Leb27FnRxo16AEHDg",
                "t2Phx4gVL4YRnNsH3jM1M7jE4Fo329E66Na",
                "t2VQZGmeNomN8c3USefeLL9nmU6M8x8CVzC",
                "t2RicCvTVTY5y9JkreSRv3Xs8q2K67YxHLi",
                "t2JrSLxTGc8wtPDe9hwbaeUjCrCfc4iZnDD",
                "t2Uh9Au1PDDSw117sAbGivKREkmMxVC5tZo",
                "t2FDwoJKLeEBMTy3oP7RLQ1Fihhvz49a3Bv",
                "t2FY18mrgtb7QLeHA8ShnxLXuW8cNQ2n1v8",
                "t2L15TkDYum7dnQRBqfvWdRe8Yw3jVy9z7g",
            };

            // ZF and MG use the same address for each funding period
            std::vector<std::string> zf_addresses(51, "t27eWDgjFYJGVXmzrXeVjnb5J3uXDM9xH9v");
            std::vector<std::string> mg_addresses(51, "t2Gvxv2uNM7hbbACjNox4H6DjByoKZ2Fa3P");

            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_ZIP214_BP,
                canopyActivation,
                2796000, // *not* the NU6 activation height
                bp_addresses);
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_ZIP214_ZF,
                canopyActivation,
                2796000, // *not* the NU6 activation height
                zf_addresses);
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_ZIP214_MG,
                canopyActivation,
                2796000, // *not* the NU6 activation height
                mg_addresses);

            // ZIP 214 Revision 1
            // FPF uses a single address repeated 13 times, once for each funding period.
            // There are 13 periods because the start height does not align with a period boundary.
            std::vector<std::string> fpf_addresses(13, "t2HifwjUj9uyxr9bknR8LFuQbc98c3vkXtu");
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_FPF_ZCG,
                nu6Activation,
                3396000,
                fpf_addresses);
            consensus.AddZIP207LockboxStream(
                keyConstants,
                Consensus::FS_DEFERRED,
                nu6Activation,
                3396000);

            // ZIP 214 Revision 2
            // FPF uses a single address repeated 27 times, once for each funding period.
            // There are 27 periods because the start height is after the second halving
            // on testnet and does not align with a period boundary.
            std::vector<std::string> fpf_addresses_h3(27, "t2HifwjUj9uyxr9bknR8LFuQbc98c3vkXtu");
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_FPF_ZCG_H3,
                nu6_1Activation,
                4476000,
                fpf_addresses_h3);
            consensus.AddZIP207LockboxStream(
                keyConstants,
                Consensus::FS_CCF_H3,
                nu6_1Activation,
                4476000);

            // ZIP 271
            // For testing purposes, we split the lockbox contents into 10 equal chunks.
            std::string nu6_1_kho_address = "t2RnBRiqrN1nW4ecZs1Fj3WWjNdnSs4kiX8";
            static const CAmount nu6_1_disbursement_amount = 78750 * COIN;
            static const CAmount nu6_1_chunk_amount = 7875 * COIN;
            static constexpr auto nu6_1_chunks = {
                Consensus::LD_ZIP271_NU6_1_CHUNK_1,
                Consensus::LD_ZIP271_NU6_1_CHUNK_2,
                Consensus::LD_ZIP271_NU6_1_CHUNK_3,
                Consensus::LD_ZIP271_NU6_1_CHUNK_4,
                Consensus::LD_ZIP271_NU6_1_CHUNK_5,
                Consensus::LD_ZIP271_NU6_1_CHUNK_6,
                Consensus::LD_ZIP271_NU6_1_CHUNK_7,
                Consensus::LD_ZIP271_NU6_1_CHUNK_8,
                Consensus::LD_ZIP271_NU6_1_CHUNK_9,
                Consensus::LD_ZIP271_NU6_1_CHUNK_10,
            };
            static_assert(nu6_1_chunk_amount * nu6_1_chunks.size() == nu6_1_disbursement_amount);
            for (auto idx : nu6_1_chunks) {
                consensus.AddZIP271LockboxDisbursement(
                    keyConstants,
                    idx,
                    Consensus::UPGRADE_NU6_1,
                    nu6_1_chunk_amount,
                    nu6_1_kho_address);
            }
        }

        // On testnet we activate this rule 6 blocks after Blossom activation. From block 299188 and
        // prior to Blossom activation, the testnet minimum-difficulty threshold was 15 minutes (i.e.
        // a minimum difficulty block can be mined if no block is mined normally within 15 minutes):
        // <https://zips.z.cash/zip-0205#change-to-difficulty-adjustment-on-testnet>
        // However the median-time-past is 6 blocks behind, and the worst-case time for 7 blocks at a
        // 15-minute spacing is ~105 minutes, which exceeds the limit imposed by the soft fork of
        // 90 minutes.
        //
        // After Blossom, the minimum difficulty threshold time is changed to 6 times the block target
        // spacing, which is 7.5 minutes:
        // <https://zips.z.cash/zip-0208#minimum-difficulty-blocks-on-the-test-network>
        // 7 times that is 52.5 minutes which is well within the limit imposed by the soft fork.

        static_assert(6 * Consensus::POST_BLOSSOM_POW_TARGET_SPACING * 7 < MAX_FUTURE_BLOCK_TIME_MTP - 60,
                      "MAX_FUTURE_BLOCK_TIME_MTP is too low given block target spacing");
        consensus.nFutureTimestampSoftForkHeight = consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight + 6;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("000000000000000000000000000000000000000000000000000000263c0984a2");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0x1a;
        pchMessageStart[2] = 0xf9;
        pchMessageStart[3] = 0xbf;
        vAlertPubKey = ParseHex("044e7a1553392325c871c5ace5d6ad73501c66f4c185d6b0453cf45dec5a1322e705c672ac1a27ef7cdaf588c10effdf50ed5f95f85f2f54a5f6159fca394ed0c6");
        nDefaultPort = 18233;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(
            1477648033,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000006"),
            ParseHex("00a6a51259c3f6732481e2d035197218b7a69504461d04335503cd69759b2d02bd2b53a9653f42cb33c608511c953673fa9da76170958115fe92157ad3bb5720d927f18e09459bf5c6072973e143e20f9bdf0584058c96b7c2234c7565f100d5eea083ba5d3dbaff9f0681799a113e7beff4a611d2b49590563109962baa149b628aae869af791f2f70bb041bd7ebfa658570917f6654a142b05e7ec0289a4f46470be7be5f693b90173eaaa6e84907170f32602204f1f4e1c04b1830116ffd0c54f0b1caa9a5698357bd8aa1f5ac8fc93b405265d824ba0e49f69dab5446653927298e6b7bdc61ee86ff31c07bde86331b4e500d42e4e50417e285502684b7966184505b885b42819a88469d1e9cf55072d7f3510f85580db689302eab377e4e11b14a91fdd0df7627efc048934f0aff8e7eb77eb17b3a95de13678004f2512293891d8baf8dde0ef69be520a58bbd6038ce899c9594cf3e30b8c3d9c7ecc832d4c19a6212747b50724e6f70f6451f78fd27b58ce43ca33b1641304a916186cfbe7dbca224f55d08530ba851e4df22baf7ab7078e9cbea46c0798b35a750f54103b0cdd08c81a6505c4932f6bfbd492a9fced31d54e98b6370d4c96600552fcf5b37780ed18c8787d03200963600db297a8f05dfa551321d17b9917edadcda51e274830749d133ad226f8bb6b94f13b4f77e67b35b71f52112ce9ba5da706ad9573584a2570a4ff25d29ab9761a06bdcf2c33638bf9baf2054825037881c14adf3816ba0cbd0fca689aad3ce16f2fe362c98f48134a9221765d939f0b49677d1c2447e56b46859f1810e2cf23e82a53e0d44f34dae932581b3b7f49eaec59af872cf9de757a964f7b33d143a36c270189508fcafe19398e4d2966948164d40556b05b7ff532f66f5d1edc41334ef742f78221dfe0c7ae2275bb3f24c89ae35f00afeea4e6ed187b866b209dc6e83b660593fce7c40e143beb07ac86c56f39e895385924667efe3a3f031938753c7764a2dbeb0a643fd359c46e614873fd0424e435fa7fac083b9a41a9d6bf7e284eee537ea7c50dd239f359941a43dc982745184bf3ee31a8dc850316aa9c6b66d6985acee814373be3458550659e1a06287c3b3b76a185c5cb93e38c1eebcf34ff072894b6430aed8d34122dafd925c46a515cca79b0269c92b301890ca6b0dc8b679cdac0f23318c105de73d7a46d16d2dad988d49c22e9963c117960bdc70ef0db6b091cf09445a516176b7f6d58ec29539166cc8a38bbff387acefffab2ea5faad0e8bb70625716ef0edf61940733c25993ea3de9f0be23d36e7cb8da10505f9dc426cd0e6e5b173ab4fff8c37e1f1fb56d1ea372013d075e0934c6919393cfc21395eea20718fad03542a4162a9ded66c814ad8320b2d7c2da3ecaf206da34c502db2096d1c46699a91dd1c432f019ad434e2c1ce507f91104f66f491fed37b225b8e0b2888c37276cfa0468fc13b8d593fd9a2675f0f5b20b8a15f8fa7558176a530d6865738ddb25d3426dab905221681cf9da0e0200eea5b2eba3ad3a5237d2a391f9074bf1779a2005cee43eec2b058511532635e0fea61664f531ac2b356f40db5c5d275a4cf5c82d468976455af4e3362cc8f71aa95e71d394aff3ead6f7101279f95bcd8a0fedce1d21cb3c9f6dd3b182fce0db5d6712981b651f29178a24119968b14783cafa713bc5f2a65205a42e4ce9dc7ba462bdb1f3e4553afc15f5f39998fdb53e7e231e3e520a46943734a007c2daa1eda9f495791657eefcac5c32833936e568d06187857ed04d7b97167ae207c5c5ae54e528c36016a984235e9c5b2f0718d7b3aa93c7822ccc772580b6599671b3c02ece8a21399abd33cfd3028790133167d0a97e7de53dc8ff"),
            0x2007ffff, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        // SHIFOCOIN: Disabled genesis assertions for custom genesis block
        // assert(consensus.hashGenesisBlock == uint256S("0x05a60a92d99d85997cce3b87616c089f6124d7342af37106edc76126334a2c38"));
        // assert(genesis.hashMerkleRoot == uint256S("0xc4eaa58879081de3c24a7b117ed2b28300e7ec4c4c1dff1d3f1268b7857a4ddb"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("z.cash", "dnsseed.testnet.z.cash")); // Zcash
        vSeeds.push_back(CDNSSeedData("zfnd.org", "testnet.seeder.zfnd.org")); // Zcash Foundation
        vSeeds.push_back(CDNSSeedData("yolo.money", "testnet.is.yolo.money")); // gtank

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;


        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, consensus.hashGenesisBlock)
            (38000, uint256S("0x001e9a2d2e2892b88e9998cf7b079b41d59dd085423a921fe8386cecc42287b8")),
            1486897419,  // * UNIX timestamp of last checkpoint block
            47163,       // * total number of transactions between genesis and last checkpoint
            715          //   total number of tx / (checkpoint block height / (24 * 24))
        };

        // Hardcoded fallback value for the Sprout shielded value pool balance
        // for nodes that have not reindexed since the introduction of monitoring
        // in #2795.
        nSproutValuePoolCheckpointHeight = 440329;
        nSproutValuePoolCheckpointBalance = 40000029096803;
        fZIP209Enabled = true;
        hashSproutValuePoolCheckpointBlock = uint256S("000a95d08ba5dcbabe881fc6471d11807bcca7df5f1795c99f3ec4580db4279b");

        // Founders reward script expects a vector of 2-of-3 multisig addresses
        vFoundersRewardAddress = {
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", "t2N9PH9Wk9xjqYg9iin1Ua3aekJqfAtE543", "t2NGQjYMQhFndDHguvUw4wZdNdsssA6K7x2", "t2ENg7hHVqqs9JwU5cgjvSbxnT2a9USNfhy",
            "t2BkYdVCHzvTJJUTx4yZB8qeegD8QsPx8bo", "t2J8q1xH1EuigJ52MfExyyjYtN3VgvshKDf", "t2Crq9mydTm37kZokC68HzT6yez3t2FBnFj", "t2EaMPUiQ1kthqcP5UEkF42CAFKJqXCkXC9",
            "t2F9dtQc63JDDyrhnfpzvVYTJcr57MkqA12", "t2LPirmnfYSZc481GgZBa6xUGcoovfytBnC", "t26xfxoSw2UV9Pe5o3C8V4YybQD4SESfxtp", "t2D3k4fNdErd66YxtvXEdft9xuLoKD7CcVo",
            "t2DWYBkxKNivdmsMiivNJzutaQGqmoRjRnL", "t2C3kFF9iQRxfc4B9zgbWo4dQLLqzqjpuGQ", "t2MnT5tzu9HSKcppRyUNwoTp8MUueuSGNaB", "t2AREsWdoW1F8EQYsScsjkgqobmgrkKeUkK",
            "t2Vf4wKcJ3ZFtLj4jezUUKkwYR92BLHn5UT", "t2K3fdViH6R5tRuXLphKyoYXyZhyWGghDNY", "t2VEn3KiKyHSGyzd3nDw6ESWtaCQHwuv9WC", "t2F8XouqdNMq6zzEvxQXHV1TjwZRHwRg8gC",
            "t2BS7Mrbaef3fA4xrmkvDisFVXVrRBnZ6Qj", "t2FuSwoLCdBVPwdZuYoHrEzxAb9qy4qjbnL", "t2SX3U8NtrT6gz5Db1AtQCSGjrpptr8JC6h", "t2V51gZNSoJ5kRL74bf9YTtbZuv8Fcqx2FH",
            "t2FyTsLjjdm4jeVwir4xzj7FAkUidbr1b4R", "t2EYbGLekmpqHyn8UBF6kqpahrYm7D6N1Le", "t2NQTrStZHtJECNFT3dUBLYA9AErxPCmkka", "t2GSWZZJzoesYxfPTWXkFn5UaxjiYxGBU2a",
            "t2RpffkzyLRevGM3w9aWdqMX6bd8uuAK3vn", "t2JzjoQqnuXtTGSN7k7yk5keURBGvYofh1d", "t2AEefc72ieTnsXKmgK2bZNckiwvZe3oPNL", "t2NNs3ZGZFsNj2wvmVd8BSwSfvETgiLrD8J",
            "t2ECCQPVcxUCSSQopdNquguEPE14HsVfcUn", "t2JabDUkG8TaqVKYfqDJ3rqkVdHKp6hwXvG", "t2FGzW5Zdc8Cy98ZKmRygsVGi6oKcmYir9n", "t2DUD8a21FtEFn42oVLp5NGbogY13uyjy9t",
            "t2UjVSd3zheHPgAkuX8WQW2CiC9xHQ8EvWp", "t2TBUAhELyHUn8i6SXYsXz5Lmy7kDzA1uT5", "t2Tz3uCyhP6eizUWDc3bGH7XUC9GQsEyQNc", "t2NysJSZtLwMLWEJ6MH3BsxRh6h27mNcsSy",
            "t2KXJVVyyrjVxxSeazbY9ksGyft4qsXUNm9", "t2J9YYtH31cveiLZzjaE4AcuwVho6qjTNzp", "t2QgvW4sP9zaGpPMH1GRzy7cpydmuRfB4AZ", "t2NDTJP9MosKpyFPHJmfjc5pGCvAU58XGa4",
            "t29pHDBWq7qN4EjwSEHg8wEqYe9pkmVrtRP", "t2Ez9KM8VJLuArcxuEkNRAkhNvidKkzXcjJ", "t2D5y7J5fpXajLbGrMBQkFg2mFN8fo3n8cX", "t2UV2wr1PTaUiybpkV3FdSdGxUJeZdZztyt",
            };
        assert(vFoundersRewardAddress.size() <= consensus.GetLastFoundersRewardBlockHeight(0));
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        keyConstants.strNetworkID = "regtest";
        strCurrencyUnits = "REG";
        keyConstants.bip44CoinType = 1;
        consensus.fCoinbaseMustBeShielded = false;
        consensus.nSubsidySlowStartInterval = 0;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_REGTEST_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = POST_BLOSSOM_HALVING_INTERVAL(Consensus::PRE_BLOSSOM_REGTEST_HALVING_INTERVAL);
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        const size_t N = 48, K = 5;
        static_assert(equihash_parameters_acceptable(N, K));
        consensus.nEquihashN = N;
        consensus.nEquihashK = K;
        consensus.powLimit = uint256S("0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f"); // if this is any larger, the for loop in GetNextWorkRequired can overflow bnTot
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 0; // Turn off adjustment down
        consensus.nPowMaxAdjustUp = 0; // Turn off adjustment up
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = 0;
        consensus.fPowNoRetargeting = true;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170003;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170006;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170008;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nProtocolVersion = 170010;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nProtocolVersion = 170012;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nProtocolVersion = 170050;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nProtocolVersion = 170110;
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nProtocolVersion = 170130;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nProtocolVersion = 0x7FFFFFFF;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.nFundingPeriodLength = consensus.nPostBlossomSubsidyHalvingInterval / 48;
        // Defined funding streams can be enabled with node config flags.

        // These prefixes are the same as the testnet prefixes
        keyConstants.base58Prefixes[PUBKEY_ADDRESS]     = {0x1D,0x25};
        keyConstants.base58Prefixes[SCRIPT_ADDRESS]     = {0x1C,0xBA};
        keyConstants.base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        keyConstants.base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        keyConstants.base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0xB6};
        keyConstants.base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        keyConstants.base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

        keyConstants.bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "zregtestsapling";
        keyConstants.bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "zviewregtestsapling";
        keyConstants.bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "zivkregtestsapling";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-regtest";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_FVK]         = "zxviewregtestsapling";

        keyConstants.bech32mHRPs[TEX_ADDRESS]                 = "texregtest";

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        pchMessageStart[0] = 0xaa;
        pchMessageStart[1] = 0xe8;
        pchMessageStart[2] = 0x3f;
        pchMessageStart[3] = 0x5f;
        nDefaultPort = 18344;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(
            1296688602,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000009"),
            ParseHex("01936b7db1eb4ac39f151b8704642d0a8bda13ec547d54cd5e43ba142fc6d8877cab07b3"),
            0x200f0f0f, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        // SHIFOCOIN: Disabled regtest genesis assertions for custom genesis block
        // assert(consensus.hashGenesisBlock == uint256S("0x029f11d80ef9765602235e1bc9727e3eb6ba20839319f761fee920d63401e327"));
        // assert(genesis.hashMerkleRoot == uint256S("0xc4eaa58879081de3c24a7b117ed2b28300e7ec4c4c1dff1d3f1268b7857a4ddb"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206")),
            0,
            0,
            0
        };

        // Founders reward script expects a vector of 2-of-3 multisig addresses
        vFoundersRewardAddress = { "t2FwcEhFdNXuFMv1tcYwaBJtYVtMj8b1uTg" };
        assert(vFoundersRewardAddress.size() <= consensus.GetLastFoundersRewardBlockHeight(0));

        // do not require the wallet backup to be confirmed in regtest mode
        fRequireWalletBackup = false;
    }

    void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
    {
        assert(idx > Consensus::BASE_SPROUT && idx < Consensus::MAX_NETWORK_UPGRADES);
        consensus.vUpgrades[idx].nActivationHeight = nActivationHeight;
    }

    void UpdateFundingStreamParameters(Consensus::FundingStreamIndex idx, Consensus::FundingStream fs)
    {
        assert(idx >= Consensus::FIRST_FUNDING_STREAM && idx < Consensus::MAX_FUNDING_STREAMS);
        consensus.vFundingStreams[idx] = fs;
    }

    void UpdateOnetimeLockboxDisbursementParameters(
        Consensus::OnetimeLockboxDisbursementIndex idx,
        Consensus::OnetimeLockboxDisbursement ld)
    {
        assert(idx >= Consensus::FIRST_ONETIME_LOCKBOX_DISBURSEMENT && idx < Consensus::MAX_ONETIME_LOCKBOX_DISBURSEMENTS);
        consensus.vOnetimeLockboxDisbursements[idx] = ld;
    }

    void UpdateRegtestPow(
        int64_t nPowMaxAdjustDown,
        int64_t nPowMaxAdjustUp,
        uint256 powLimit,
        bool noRetargeting)
    {
        consensus.nPowMaxAdjustDown = nPowMaxAdjustDown;
        consensus.nPowMaxAdjustUp = nPowMaxAdjustUp;
        consensus.powLimit = powLimit;
        consensus.fPowNoRetargeting = noRetargeting;
    }

    void SetRegTestZIP209Enabled() {
        fZIP209Enabled = true;
    }
};
static CRegTestParams regTestParams;

static const CChainParams* pCurrentParams = nullptr;

const CChainParams& Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

const CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);

    // Some python qa rpc tests need to enforce the coinbase consensus rule
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-regtestshieldcoinbase")) {
        regTestParams.SetRegTestCoinbaseMustBeShielded();
    }

    // When a developer is debugging turnstile violations in regtest mode, enable ZIP209
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-developersetpoolsizezero")) {
        regTestParams.SetRegTestZIP209Enabled();
    }
}


// Block height must be >0 and <=last founders reward block height
// Index variable i ranges from 0 - (vFoundersRewardAddress.size()-1)
std::string CChainParams::GetFoundersRewardAddressAtHeight(int nHeight) const {
    int preBlossomMaxHeight = consensus.GetLastFoundersRewardBlockHeight(0);
    // zip208
    // FounderAddressAdjustedHeight(height) :=
    // height, if not IsBlossomActivated(height)
    // BlossomActivationHeight + floor((height - BlossomActivationHeight) / BlossomPoWTargetSpacingRatio), otherwise
    bool blossomActive = consensus.NetworkUpgradeActive(nHeight, Consensus::UPGRADE_BLOSSOM);
    if (blossomActive) {
        int blossomActivationHeight = consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight;
        nHeight = blossomActivationHeight + ((nHeight - blossomActivationHeight) / Consensus::BLOSSOM_POW_TARGET_SPACING_RATIO);
    }
    assert(nHeight > 0 && nHeight <= preBlossomMaxHeight);
    size_t addressChangeInterval = (preBlossomMaxHeight + vFoundersRewardAddress.size()) / vFoundersRewardAddress.size();
    size_t i = nHeight / addressChangeInterval;
    return vFoundersRewardAddress[i];
}

// Block height must be >0 and <=last founders reward block height
// The founders reward address is expected to be a multisig (P2SH) address
CScript CChainParams::GetFoundersRewardScriptAtHeight(int nHeight) const {
    assert(nHeight > 0 && nHeight <= consensus.GetLastFoundersRewardBlockHeight(nHeight));

    KeyIO keyIO(*this);
    auto address = keyIO.DecodePaymentAddress(GetFoundersRewardAddressAtHeight(nHeight).c_str());
    assert(address.has_value());
    assert(std::holds_alternative<CScriptID>(address.value()));
    CScriptID scriptID = std::get<CScriptID>(address.value());
    CScript script = CScript() << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
    return script;
}

std::string CChainParams::GetFoundersRewardAddressAtIndex(int i) const {
    assert(i >= 0 && i < vFoundersRewardAddress.size());
    return vFoundersRewardAddress[i];
}

void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    regTestParams.UpdateNetworkUpgradeParameters(idx, nActivationHeight);
}

void UpdateFundingStreamParameters(Consensus::FundingStreamIndex idx, Consensus::FundingStream fs)
{
    regTestParams.UpdateFundingStreamParameters(idx, fs);
}

void UpdateOnetimeLockboxDisbursementParameters(
    Consensus::OnetimeLockboxDisbursementIndex idx,
    Consensus::OnetimeLockboxDisbursement ld)
{
    regTestParams.UpdateOnetimeLockboxDisbursementParameters(idx, ld);
}

void UpdateRegtestPow(
    int64_t nPowMaxAdjustDown,
    int64_t nPowMaxAdjustUp,
    uint256 powLimit,
    bool noRetargeting)
{
    regTestParams.UpdateRegtestPow(nPowMaxAdjustDown, nPowMaxAdjustUp, powLimit, noRetargeting);
}
