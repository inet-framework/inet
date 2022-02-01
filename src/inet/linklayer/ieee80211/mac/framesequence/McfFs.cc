//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/framesequence/McfFs.h"

#include "inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.h"

namespace inet {
namespace ieee80211 {

McfFs::McfFs() :
    // Excerpt from G.3 EDCA and HCCA sequences
    // mcf-sequence =
    //   ( [ CTS ] |{( Data + group + QoS ) | Management + broadcast } ) | ( [ CTS ] 1{txop-sequence} ) |
    //   group-mccaop-abandon;
    AlternativesFs({/* TODO */},
                   ALTERNATIVESFS_SELECTOR(selectMcfSequence))
{
}

int McfFs::selectMcfSequence(AlternativesFs *frameSequence, FrameSequenceContext *context)
{
    return -1;
}

} // namespace ieee80211
} // namespace inet

