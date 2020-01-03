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

#ifndef __INET_IPASSIVEPACKETSINK_H
#define __INET_IPASSIVEPACKETSINK_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for passive packet sinks.
 */
class INET_API IPassivePacketSink
{
  public:
    virtual ~IPassivePacketSink() {}

    /**
     * Returns false if the packet sink is full at the given gate and no more
     * packets can be pushed into it without raising an error. The gate must
     * support pushing packets.
     */
    virtual bool canPushSomePacket(cGate *gate = nullptr) const = 0;

    /**
     * Returns true if the given packet can be pushed at the given gate into
     * the packet sink. The packet must not be nullptr and the gate must support
     * pushing packets.
     */
    virtual bool canPushPacket(Packet *packet, cGate *gate = nullptr) const = 0;

    /**
     * Pushes a packet into the packet sink at the given gate. The consumer must
     * not be full at the gate. The packet must not be nullptr and the gate
     * must support pushing packets.
     */
    virtual void pushPacket(Packet *packet, cGate *gate = nullptr) = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPASSIVEPACKETSINK_H

