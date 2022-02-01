//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/framesequence/HtTxOpFs.h"

#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"

namespace inet {
namespace ieee80211 {

HtTxOpFs::HtTxOpFs() :
    // G.4 HT sequences
    // ht-txop-sequence =
    //   L-sig-protected-sequence |
    //   ht-nav-protected-sequence |
    //   dual-cts-protected-sequence |
    //   1 {initiator-sequence};
    AlternativesFs({},
                   ALTERNATIVESFS_SELECTOR(selectHtTxOpSequence))
{
}

int HtTxOpFs::selectHtTxOpSequence(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    return 0;
}

} // namespace ieee80211
} // namespace inet

