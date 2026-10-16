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
 * This class defines the interface for packet queues. A packet queue is a
 * passive packet sink and a passive packet source. enqueuePacket() is the
 * form of pushPacket() at the input gate without the gate argument, and
 * dequeuePacket(), with or without a filter, is the same form of pullPacket()
 * at the output gate.
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

    /**
     * Dequeues the packet that dequeuePacket() would dequeue if the queue held
     * only the packets that the filter matches. The effects are the effects of
     * dequeuePacket() for that packet. The ownership of the packet is
     * transferred from the queue to the caller.
     *
     * The queue must hold a packet that the filter matches. The returned packet
     * must not be nullptr. The filter must not be nullptr, and it must not
     * change the state of the simulation.
     */
    virtual Packet *dequeuePacket(const IPacketFilterFunction *filter) = 0;
};

} // namespace queueing
} // namespace inet

#endif

