//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/framesequence/PcfFs.h"

#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"

namespace inet {
namespace ieee80211 {

PcfFs::PcfFs() :
    // Excerpt from G.2 Basic sequences (p. 2309)
    // frame-sequence =
    //   ( PS-Poll ACK ) |
    //   ( [ Beacon + DTIM ] {cf-sequence} [ CF-End [+ CF-Ack ] ] )
    AlternativesFs({new SequentialFs({}), // TODO poll
                    new SequentialFs({})}, // TODO beacon
                   ALTERNATIVESFS_SELECTOR(selectPcfSequence))
{
}

int PcfFs::selectPcfSequence(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    return 0;
}

} // namespace ieee80211
} // namespace inet

