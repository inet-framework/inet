//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETCOMPARATORFUNCTION_H
#define __INET_IPACKETCOMPARATORFUNCTION_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet comparator functions.
 */
class INET_API IPacketComparatorFunction : public cQueue::Comparator
{
  public:
    virtual ~IPacketComparatorFunction() {}

    /**
     * Returns the order of the given packets.
     */
    virtual int comparePackets(Packet *packet1, Packet *packet2) const = 0;

    // Implements cQueue::Comparator interface
    virtual bool less(cObject *a, cObject *b) override { return comparePackets(check_and_cast<Packet *>(a), check_and_cast<Packet *>(b)) < 0; }
};

} // namespace queueing
} // namespace inet

#endif

