//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPACKETQUEUE_H
#define __INET_IPACKETQUEUE_H

#include "inet/queueing/contract/IPacketCollection.h"
#include "inet/queueing/contract/IPacketExtractor.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet queues.
 */
class INET_API IPacketQueue : public virtual IPacketCollection, public virtual IPacketExtractor, public virtual IPassivePacketSink, public virtual IPassivePacketSource
{
  public:
    enum class PacketRemovalReason {
        DEQUEUED,  // Normal queue processing transferred ownership out of the queue.
        REMOVED,   // Explicit removal outside normal queue processing.
        DROPPED,   // The queue destructively removed the packet.
    };

    /**
     * Emitted once per logical departure, after detachment and before transfer/deletion.
     * The Packet and PacketQueueRemovalDetails are borrowed for synchronous delivery.
     * Listeners must filter the source to their subscribed logical queue.
     */
    static simsignal_t packetQueueDepartureSignal;

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

    virtual Packet *findPacket(const PacketPredicate& predicate) const = 0;

    /**
     * Dequeues the first matching packet according to the queue provider's
     * scheduling policy. Ownership is transferred to the caller.
     */
    virtual Packet *dequeuePacket(const PacketPredicate& predicate) = 0;
};

} // namespace queueing
} // namespace inet

#endif
