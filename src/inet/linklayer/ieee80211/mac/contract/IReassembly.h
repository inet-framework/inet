//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IREASSEMBLY_H
#define __INET_IREASSEMBLY_H

#include "inet/linklayer/common/MacAddress.h"

namespace inet {
namespace ieee80211 {

class Ieee80211DataOrMgmtHeader;

/**
 * Abstract interface for classes that encapsulate the functionality of
 * reassembling frames from fragments. Fragmentation reassembly classes
 * are typically instantiated as part of an UpperMac.
 */
class INET_API IReassembly
{
  public:
    virtual ~IReassembly() {}

    /**
     * Add a fragment to the reassembly buffer. If the new fragment completes a frame,
     * then the reassembled frame is returned (and fragments are removed from the buffer),
     * otherwise the function returns nullptr.
     */
    virtual Packet *addFragment(Packet *frame) = 0;

    /**
     * Discard fragments from the reassembly buffer. Frames are identified by the transmitter
     * address, the TID, and the sequence number range [startSeqNumber, endSeqNumber[.
     * Set tid=-1 for non-QoS frames.
     */
    virtual void purge(const MacAddress& address, int tid, int startSeqNumber, int endSeqNumber) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

