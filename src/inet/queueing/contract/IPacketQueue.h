//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

#endif

