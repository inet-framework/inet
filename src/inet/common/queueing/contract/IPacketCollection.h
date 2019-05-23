//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
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
     */
    virtual int getMaxNumPackets() = 0;

    /**
     * Returns the number of available packets in the collection.
     */
    virtual int getNumPackets() = 0;

    /**
     * Returns maximum allowed total length of all packets in the collection.
     */
    virtual b getMaxTotalLength() = 0;

    /**
     * Returns the total length of all packets in the collection.
     */
    virtual b getTotalLength() = 0;

    /**
     * Returns the packet at the given index.
     * Throws error when index is out of range.
     */
    virtual Packet *getPacket(int index) = 0;

    /**
     * Returns true if there are no packets available in the collection.
     */
    virtual bool isEmpty() = 0;

    /**
     * Removes a packet from the collection. The collection must contain the packet.
     */
    virtual void removePacket(Packet *packet) = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPACKETCOLLECTION_H

