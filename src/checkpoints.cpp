// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints = boost::assign::map_list_of
            (0, uint256("0x746f76f5ec55d46119b3f1940b3518e323f708dc300636dfd8be3a66393a8841"))
            (1, uint256("0x11dddba60df22a379524d8e5135ae802d6c5062ac5ce80e303d5108c7a726cbc"))
            (2, uint256("0x78dd9c228a6c101e3648b01f737b0aea5c18167f563f36d902e6a3da83241964"))
            (3, uint256("0x15ff9b24ab903cb7cbc3ab447a55cd6b8d9b92c3e50d9ffba90c714836befa7e"))
            (10, uint256("0x1dc0af4a6f8cf3ddb7529d56890358dfaf10becd504cdf1a22d88ba98cc94369"))
            (100, uint256("0xc0328772dac90497ada3ebb903c4af24673a3761bc916a05b483e45447ac3c03"))
            (400, uint256("0xe58dd48e08feea104972f327aa30ae1870b88f491f58c43ec6e816d8fd6c6870"))
            (800, uint256("0xc1c762fb07d07b6b0bf8effaa1d5424322c2baf4d51a65b646d4dcd88df50e50"))
            (840, uint256("0x55b26f1d4832ef30df72f45c2b8ff88848ab6549f8c4b24bb3429287366f3da5"))
            (841, uint256("0x6164d72c4ba04cb9dda81922616dc09d44d87849879a226453ca5843439c1f13"))
            (960, uint256("0xe45816c1f8139d416ddc5f6ce62bbf2d1128b4d046855b079ca58d1beb576307"))
            (5000, uint256("0xf53ce360cdcb4a1ad539c7431bf94f6d1cd24a3722df738b7e1fdad83b2ff105"))
            (10000, uint256("0xe25b900540de61c3ff165abe351d7cfba907f387feb46296b3f7ef1de05217d0"))
            (15000, uint256("0xd960447e8816a139b183edf294e3031c912d7bf6ac0f825b624809bf334587f2"))
            (20000, uint256("0xe2a35d44db73b6e1a5e2fc09ba1e524ef5a53f26c2874d417aa9b730ca98ae83"))
            (25000, uint256("0xb014a9f4b75973bba2527f349284cc9b2a83dc3f78efc3952246ab5cf53e8ae6"))
            (30000, uint256("0x6e66adfa8d8c406b865f75b399eb5c1341daa54b5e7371ee8e0fe6ad80f88aa4"))
            (34560, uint256("0xdd285d2c8864638ccb4acd744a4264df8cf8ff2653944d47ae8a74035103cf7f"))
            (116000, uint256("0xd7da813a04b6b53b4e41cc3cff4f5f6a288dfc1298571733618beb6e5e139038"))
            ;

    static const CCheckpointData data = {
        &mapCheckpoints,
       1516122911, // * UNIX timestamp of last checkpoint block
       126640,    // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
       300000.0     // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet = boost::assign::map_list_of (0, uint256("0x"));
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        //0,
        //0,
        //0.0
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        // Javacoin has no checkpoints yet!
        return true;
        if (fTestNet) return true; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        // Javacoin has no checkpoints yet!
        return 0;
        if (fTestNet) return 0; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        // Javacoin has no checkpoints yet!
        return NULL;
        if (fTestNet) return NULL; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
