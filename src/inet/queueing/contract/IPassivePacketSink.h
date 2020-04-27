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
     * Returns false if the packet sink is full at the given gate and no more
     * packets can be pushed into it without raising an error.
     *
     * The gate must be a valid gate of this module and it must support pushing
     * packets.
     */
    virtual bool canPushSomePacket(cGate *gate) const = 0;

    /**
     * Returns true if the given packet can be pushed at the given gate into
     * the packet sink without raising an error.
     *
     * The packet must not be nullptr. The gate must be a valid gate of this
     * module and it must support pushing packets.
     */
    virtual bool canPushPacket(Packet *packet, cGate *gate) const = 0;

    /**
     * Pushes the packet into the packet sink at the given gate. This operation
     * pushes the packet as a whole. The onwership of the packet is transferred
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
     * a packet streaming operation. The onwership of the packet is transferred
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
     * a packet streaming operation. The onwership of the packet is transferred
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
    virtual void pushPacketEnd(Packet *packet, cGate *gate, bps datarate) = 0;

    /**
     * Progresses pushing the packet into the packet sink at the given gate. This
     * is a packet streaming operation. The position specifies where the packet
     * streaming is at the moment. The extra length parameter partially fixes
     * the future of the packet streaming operation. The onwership of the packet
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

    /**
     * Returns the processed length of the currently streaming packet.
     *
     * The packet must not be nullptr and the gate must support pushing and
     * streaming packets.
     */
    virtual b getPushPacketProcessedLength(Packet *packet, cGate *gate) = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPASSIVEPACKETSINK_H

