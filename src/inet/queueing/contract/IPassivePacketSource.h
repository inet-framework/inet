//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPASSIVEPACKETSOURCE_H
#define __INET_IPASSIVEPACKETSOURCE_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for passive packet sources.
 * See the corresponding NED file for more details.
 */
class INET_API IPassivePacketSource
{
  public:
    virtual ~IPassivePacketSource() {}

    /**
     * Returns true if packets can be pulled at the given gate from the packet
     * source.
     *
     * The gate must be a valid gate of this module and it must support pulling
     * packets.
     */
    virtual bool canPullSomePacket(cGate *gate) const = 0;

    /**
     * Returns the packet that can be pulled at the given gate. The returned
     * value is nullptr if there is no such packet.
     *
     * The gate must be a valid gate of this module and it must support pushing
     * packets.
     */
    virtual Packet *canPullPacket(cGate *gate) const = 0;

    /**
     * Pulls the packet from the packet source at the given gate. This operation
     * pulls the packet as a whole. The ownership of the packet is transferred
     * to the sink.
     *
     * The source must not be empty at the given gate. The returned packet must
     * not be nullptr. The gate must be a valid gate of this module and it must
     * support pulling and passing packets.
     */
    virtual Packet *pullPacket(cGate *gate) = 0;

    /**
     * Starts pulling the packet from the packet source at the given gate. This
     * is a packet streaming operation. The ownership of the packet is transferred
     * to the sink.
     *
     * Packet streaming can be started with any of the streaming operations, and
     * ends when the streaming position plus the extra processable packet length
     * equals to the total packet length.
     *
     * This method is called, for example, when a preemption supporting server
     * module starts streaming a packet from the source.
     *
     * The source must not be empty at the gate and no other packet streaming
     * can be in progress. The gate must be a valid gate of this module and it
     * must support pulling and streaming packets. The returned packet must not
     * be nullptr.
     */
    virtual Packet *pullPacketStart(cGate *gate, bps datarate) = 0;

    /**
     * Ends pulling the packet from the packet source at the given gate. This is
     * a packet streaming operation. The ownership of the packet is transferred
     * to the sink.
     *
     * Packet streaming can be started with any of the streaming operations, and
     * ends when the streaming position plus the extra processable packet length
     * equals to the total packet length.
     *
     * This method is called, for example, when a preemption supporting server
     * module ends streaming a packet from the the source.
     *
     * The source must not be empty at the gate and no other packet streaming
     * can be in progress. The gate must be a valid gate of this module and it
     * must support pulling and streaming packets. The returned packet must not
     * be nullptr.
     */
    virtual Packet *pullPacketEnd(cGate *gate) = 0;

    /**
     * Progresses pulling the packet from the packet source at the given gate.
     * This is a packet streaming operation. The position specifies where the
     * packet streaming is at the moment. The extra length parameter partially
     * fixes the future of the packet streaming operation. The ownership of the
     * packet is transferred to the sink.
     *
     * Packet streaming can be started with any of the streaming operations, and
     * ends when the streaming position plus the extra processable packet length
     * equals to the total packet length.
     *
     * This method is called, for example, to notify the source about a change
     * in the packet data when a preemption occurs.
     *
     * The source must not be empty at the gate and no other packet streaming
     * can be in progress. The gate must be a valid gate of this module and it
     * must support pulling and streaming packets. The returned packet must not
     * be nullptr.
     */
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) = 0;
};

} // namespace queueing
} // namespace inet

#endif

