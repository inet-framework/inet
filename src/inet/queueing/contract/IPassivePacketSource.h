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

#ifndef __INET_IPASSIVEPACKETSOURCE_H
#define __INET_IPASSIVEPACKETSOURCE_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for passive packet sources.
 */
class INET_API IPassivePacketSource
{
  public:
    virtual ~IPassivePacketSource() {}

    /**
     * Returns false if the packet source is empty at the given gate and no more
     * packets can be pulled without raising an error. The gate must support
     * pulling packets.
     */
    virtual bool canPullSomePacket(cGate *gate = nullptr) const = 0;

    /**
     * Returns the packet that can be pulled at the given gate. The returned
     * value may be nullptr if there is no such packet. The gate must support
     * pulling packets.
     */
    virtual Packet *canPullPacket(cGate *gate = nullptr) const = 0;

    /**
     * Pops a packet from the packet source at the given gate. The provider must
     * not be empty at the given gate. The returned packet is never nullptr, and
     * the gate must support pulling packets.
     */
    virtual Packet *pullPacket(cGate *gate = nullptr) = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPASSIVEPACKETSOURCE_H

