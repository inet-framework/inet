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

#ifndef __INET_IPACKETBUFFER_H
#define __INET_IPACKETBUFFER_H

#include "inet/common/packet/Packet.h"
#include "inet/queueing/contract/IPacketCollection.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet buffers.
 */
class INET_API IPacketBuffer : public virtual IPacketCollection
{
  public:
    class INET_API ICallback
    {
      public:
        /**
         * Notifies the packet owner about the packet being removed from the buffer.
         * The packet is never nullptr.
         */
        virtual void handlePacketRemoved(Packet *packet) = 0;
    };

  public:
    virtual ~IPacketBuffer() {}

    /**
     * Adds the packet to the buffer.
     */
    virtual void addPacket(Packet *packet) = 0;

    /**
     * Removes the packet from the buffer.
     */
    virtual void removePacket(Packet *packet) = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPACKETBUFFER_H

