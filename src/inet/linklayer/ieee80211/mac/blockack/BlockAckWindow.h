//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BLOCKACKWINDOW_H
#define __INET_BLOCKACKWINDOW_H

#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"

namespace inet {
namespace ieee80211 {

/**
 * Shared cyclic-window decisions for Block Ack receive state.
 *
 * The start and size remain owned by the Block Ack record or receive buffer;
 * this class only keeps the sequence-space predicates in one place.
 */
class INET_API BlockAckWindow
{
  public:
    static constexpr int HALF_SEQUENCE_SPACE = 2048;

    static bool isBefore(SequenceNumberCyclic sequenceNumber, SequenceNumberCyclic reference)
    {
        return sequenceNumber < reference;
    }

    static bool isAtOrAfter(SequenceNumberCyclic sequenceNumber, SequenceNumberCyclic reference)
    {
        return reference <= sequenceNumber;
    }

    static bool isAfter(SequenceNumberCyclic sequenceNumber, SequenceNumberCyclic reference)
    {
        return reference < sequenceNumber;
    }

    static bool isWithin(SequenceNumberCyclic startingSequenceNumber, int windowSize, SequenceNumberCyclic sequenceNumber)
    {
        return isAtOrAfter(sequenceNumber, startingSequenceNumber) && isBefore(sequenceNumber, startingSequenceNumber + windowSize);
    }

    static bool isBeyond(SequenceNumberCyclic startingSequenceNumber, int windowSize, SequenceNumberCyclic sequenceNumber)
    {
        return isAtOrAfter(sequenceNumber, startingSequenceNumber + windowSize) && isBefore(sequenceNumber, startingSequenceNumber + HALF_SEQUENCE_SPACE);
    }

    static SequenceNumberCyclic getStartingSequenceNumber(SequenceNumberCyclic sequenceNumber, int windowSize)
    {
        return sequenceNumber - windowSize + 1;
    }
};

} // namespace ieee80211
} // namespace inet

#endif
