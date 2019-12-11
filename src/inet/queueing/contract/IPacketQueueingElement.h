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

#ifndef __INET_IPACKETQUEUEINGELEMENT_H
#define __INET_IPACKETQUEUEINGELEMENT_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet queueing elements.
 */
class INET_API IPacketQueueingElement
{
  public:
    virtual ~IPacketQueueingElement() {}

    /**
     * Returns true if the element supports pushing packets at the given gate.
     * For output gates, true means that this module must push packets into the
     * connected module. For input gates, true means that the connected module
     * must push packets into this module. Connecting incompatible gates raises
     * an error during initialize. The gate must not be nullptr.
     */
    virtual bool supportsPushPacket(cGate *gate) const = 0;

    /**
     * Returns true if the element supports popping packets at the given gate.
     * For output gates, true means that the connected module must pop packets
     * from this module. For input gates, true means that this module must pop
     * packets from the connected module. Connecting incompatible gates raises
     * an error during initialize. The gate must not be nullptr.
     */
    virtual bool supportsPopPacket(cGate *gate) const = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPACKETQUEUEINGELEMENT_H

