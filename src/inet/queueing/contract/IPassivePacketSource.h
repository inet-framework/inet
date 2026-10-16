//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IPASSIVEPACKETSOURCE_H
#define __INET_IPASSIVEPACKETSOURCE_H

#include "inet/common/packet/Packet.h"
#include "inet/queueing/contract/IPacketFilterFunction.h"

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
     * This function is a query: it must not change the state of the
     * simulation, so any number of calls in any order gives the same results.
     *
     * The gate must be a valid gate of this module and it must support pulling
     * packets.
     */
    virtual bool canPullSomePacket(const cGate *gate) const = 0;

    /**
     * Returns the packet that can be pulled at the given gate. The returned
     * value is nullptr if there is no such packet.
     *
     * This function is a query: it must not change the state of the
     * simulation. If it returns a packet, then pullPacket() at the same gate,
     * in the same event and with no other operation between the two calls,
     * returns that packet.
     *
     * The gate must be a valid gate of this module and it must support pulling
     * packets.
     */
    virtual Packet *canPullPacket(const cGate *gate) const = 0;

    /**
     * Pulls the packet from the packet source at the given gate. This operation
     * pulls the packet as a whole. The ownership of the packet is transferred
     * to the sink.
     *
     * The source must not be empty at the given gate. The returned packet must
     * not be nullptr. The gate must be a valid gate of this module and it must
     * support pulling and passing packets.
     */
    virtual Packet *pullPacket(const cGate *gate) = 0;

    /**
     * Returns the packet that can be pulled at the given gate when the sink
     * accepts only the packets that the filter matches. The returned value is
     * nullptr if there is no such packet. The filter is the acceptance
     * condition of the sink, and the result is the packet that canPullPacket()
     * would return if the source held only the packets that the filter matches.
     *
     * This function is a query: it must not change the state of the
     * simulation. If it returns a packet, then pullPacket() with the same gate
     * and filter, in the same event and with no other operation between the
     * two calls, returns that packet.
     *
     * A source that cannot choose among the packets that the filter matches
     * throws an error. The gate must be a valid gate of this module and it must
     * support pulling packets. The filter must not be nullptr, and it must not
     * change the state of the simulation.
     */
    virtual Packet *canPullPacket(const cGate *gate, const IPacketFilterFunction *filter) const = 0;

    /**
     * Pulls the packet that canPullPacket() returns for the same gate and
     * filter. This operation pulls the packet as a whole, with the effects of
     * pullPacket() for that packet. The ownership of the packet is transferred
     * to the sink.
     *
     * The source must hold a packet that the filter matches. The returned
     * packet must not be nullptr. The other conditions are the same as for
     * canPullPacket() with a filter, and the gate must also support passing
     * packets.
     */
    virtual Packet *pullPacket(const cGate *gate, const IPacketFilterFunction *filter) = 0;

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
    virtual Packet *pullPacketStart(const cGate *gate, bps datarate) = 0;

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
    virtual Packet *pullPacketEnd(const cGate *gate) = 0;

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
    virtual Packet *pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength) = 0;
};

} // namespace queueing
} // namespace inet

#endif

