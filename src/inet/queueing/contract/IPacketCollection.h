//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETCOLLECTION_H
#define __INET_IPACKETCOLLECTION_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet collections.
 */
class INET_API IPacketCollection
{
  public:
    /**
     * Returns maximum allowed number of packets in the collection.
     * The value -1 means no such limit.
     */
    virtual int getMaxNumPackets() const = 0;

    /**
     * Returns the number of available packets in the collection in the range [0, inf).
     */
    virtual int getNumPackets() const = 0;

    /**
     * Returns maximum allowed total length of all packets in the collection.
     * The value -1 means no such limit.
     */
    virtual b getMaxTotalLength() const = 0;

    /**
     * Returns the total length of all packets in the collection in the range [0, inf).
     */
    virtual b getTotalLength() const = 0;

    /**
     * Returns the packet at the given index. Throws error when the index is out of range.
     */
    virtual Packet *getPacket(int index) const = 0;

    /**
     * Returns true if there are no packets available in the collection.
     */
    virtual bool isEmpty() const = 0;

    /**
     * Removes a packet from the collection. The collection must contain the packet.
     */
    virtual void removePacket(Packet *packet) = 0;

    /**
     * Removes all packets from the collection.
     */
    virtual void removeAllPackets() = 0;
};

} // namespace queueing
} // namespace inet

#endif

