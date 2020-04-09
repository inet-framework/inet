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

#ifndef __INET_IPACKETQUEUE_H
#define __INET_IPACKETQUEUE_H

#include "inet/queueing/contract/IPacketCollection.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet queues.
 */
class INET_API IPacketQueue : public virtual IPacketCollection, public virtual IPassivePacketSink, public virtual IPassivePacketSource
{
  public:
    /**
     * Enqueues the packet into the packet queue. The onwership of the packet
     * is transferred from the caller to the queue.
     *
     * The queue must not be full. The packet must not be nullptr.
     */
    virtual void enqueuePacket(Packet *packet) = 0;

    /**
     * Dequeues the packet from the packet queue. The onwership of the packet
     * is transferred from the queue to the caller.
     *
     * The queue must not be empty. The returned packet must not be nullptr.
     */
    virtual Packet *dequeuePacket() = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPACKETQUEUE_H

