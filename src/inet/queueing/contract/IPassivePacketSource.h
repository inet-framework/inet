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
 * See the corresponding NED file for more details.
 */
class INET_API IPassivePacketSource
{
  public:
    virtual ~IPassivePacketSource() {}

    /**
     * Returns false if the packet source is empty at the given gate and no more
     * packets can be pulled from it without raising an error.
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
     * pulls the packet as a whole. The onwership of the packet is transferred
     * to the sink.
     *
     * The source must not be empty at the given gate. The returned packet must
     * not be nullptr. The gate must be a valid gate of this module and it must
     * support pulling and passing packets.
     */
    virtual Packet *pullPacket(cGate *gate) = 0;

    /**
     * Starts pulling the packet from the packet source at the given gate. This
     * is a packet streaming operation. The onwership of the packet is transferred
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
    virtual Packet *pullPacketStart(cGate *gate) = 0;

    /**
     * Ends pulling the packet from the packet source at the given gate. This is
     * a packet streaming operation. The onwership of the packet is transferred
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
     * fixes the future of the packet streaming operation. The onwership of the
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
    virtual Packet *pullPacketProgress(cGate *gate, b& position, b& extraProcessableLength) = 0;

    /**
     * Returns the processed length of the currently streaming packet.
     *
     * The packet must not be nullptr and the gate must support pulling and
     * streaming packets.
     */
    virtual b getPullPacketProcessedLength(Packet *packet, cGate *gate) = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPASSIVEPACKETSOURCE_H

