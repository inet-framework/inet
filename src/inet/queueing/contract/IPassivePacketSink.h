//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPASSIVEPACKETSINK_H
#define __INET_IPASSIVEPACKETSINK_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for passive packet sinks.
 * See the corresponding NED file for more details.
 */
class INET_API IPassivePacketSink
{
  public:
    virtual ~IPassivePacketSink() {}

    /**
     * Returns true if packets can be pushed at the given gate into the packet
     * sink.
     *
     * The gate must be a valid gate of this module and it must support pushing
     * packets.
     */
    virtual bool canPushSomePacket(cGate *gate) const = 0;

    /**
     * Returns true if the given packet can be pushed at the given gate into
     * the packet sink.
     *
     * The packet must not be nullptr. The gate must be a valid gate of this
     * module and it must support pushing packets.
     */
    virtual bool canPushPacket(Packet *packet, cGate *gate) const = 0;

    /**
     * Pushes the packet into the packet sink at the given gate. This operation
     * pushes the packet as a whole. The ownership of the packet is transferred
     * to the sink.
     *
     * This method is called, for example, when a packet source module pushes a
     * packet into a queue module.
     *
     * The sink must not be full at the gate. The packet must not be nullptr.
     * The gate must be a valid gate of this module and it must support pushing
     * and passing packets.
     */
    virtual void pushPacket(Packet *packet, cGate *gate) = 0;

    /**
     * Starts pushing the packet into the packet sink at the given gate. This is
     * a packet streaming operation. The ownership of the packet is transferred
     * to the sink.
     *
     * Packet streaming can be started with any of the streaming operations, and
     * ends when the streaming position plus the extra processable packet length
     * equals to the total packet length.
     *
     * This method is called, for example, when a preemption supporting server
     * module starts streaming a packet to the sink.
     *
     * The sink must not be full at the gate and no other packet streaming can
     * be in progress. The packet must not be nullptr. The gate must be a valid
     * gate of this module and it must support pushing and streaming packets.
     */
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) = 0;

    /**
     * Ends pushing the packet into the packet sink at the given gate. This is
     * a packet streaming operation. The ownership of the packet is transferred
     * to the sink.
     *
     * Packet streaming can be started with any of the streaming operations, and
     * ends when the streaming position plus the extra processable packet length
     * equals to the total packet length.
     *
     * This method is called, for example, when a preemption supporting server
     * module ends streaming a packet to the sink.
     *
     * The sink must not be full at the gate and no other packet streaming can
     * be in progress. The packet must not be nullptr. The gate must be a valid
     * gate of this module and it must support pushing and streaming packets.
     */
    virtual void pushPacketEnd(Packet *packet, cGate *gate) = 0;

    /**
     * Progresses pushing the packet into the packet sink at the given gate. This
     * is a packet streaming operation. The position specifies where the packet
     * streaming is at the moment. The extra length parameter partially fixes
     * the future of the packet streaming operation. The ownership of the packet
     * is transferred to the sink.
     *
     * Packet streaming can be started with any of the streaming operations, and
     * ends when the streaming position plus the extra processable packet length
     * equals to the total packet length.
     *
     * This method is called, for example, to notify the sink about a change in
     * the packet data when a preemption occurs.
     *
     * The sink must not be full at the gate and no other packet streaming can
     * be in progress. The packet must not be nullptr. The gate must be a valid
     * gate of this module and it must support pushing and streaming packets.
     */
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) = 0;
};

} // namespace queueing
} // namespace inet

#endif

