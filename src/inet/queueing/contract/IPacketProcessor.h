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

#ifndef __INET_IPACKETPROCESSOR_H
#define __INET_IPACKETPROCESSOR_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for packet processors.
 */
class INET_API IPacketProcessor
{
  public:
    virtual ~IPacketProcessor() {}

    /**
     * Returns true if the processor supports sending packets at the given gate.
     * Sending a packet is an asynchronous operation that is initiated by the
     * source module. A sent packet is always passed as a whole using standard
     * handleMessage().
     *
     * For output gates, true means that this module can send packets into the
     * connected module. For input gates, true means that the connected module
     * can send packets into this module. For example, a standard OMNeT++ module
     * can send packets to an INET packet processor module, or an INET packet
     * processor module can send packets to a standard OMNeT++ module.
     *
     * Connecting incompatible gates raises an error during initialize. The gate
     * parameter must be a valid gate of this module. The gate should be marked
     * with @labels(send) in the NED file.
     */
    virtual bool supportsPacketSending(cGate *gate) const = 0;

    /**
     * Returns true if the processor supports pushing packets at the given gate.
     * Pushing a packet is a synchronous operation that is initiated by the
     * source module. A pushed packet can be passed as a whole using pushPacket(),
     * or it can be streamed from the source to the sink using pushPacketStart(),
     * pushPacketEnd(), and pushPacketProgress().
     *
     * For output gates, true means that this module can push packets into the
     * connected module. For input gates, true means that the connected module
     * can push packets into this module. For example, a packet generator module
     * can push packets into a queue module.
     *
     * Connecting incompatible gates raises an error during initialize. The gate
     * parameter must be a valid gate of this module. The gate should be marked
     * with @labels(push) in the NED file.
     */
    virtual bool supportsPacketPushing(cGate *gate) const = 0;

    /**
     * Returns true if the processor supports pulling packets at the given gate.
     * Pulling a packet is a synchronous operation that is initiated by the
     * sink module. A pulled packet can be passed as a whole using pullPacket(),
     * or it can be streamed from the source to the sink using pullPacketStart(),
     * pullPacketEnd(), and pullPacketProgress().
     *
     * For output gates, true means that the connected module can pull packets
     * from this module. For input gates, true means that this module can pull
     * packets from the connected module. For example, a packet server module
     * can pull packets from a queue module.
     *
     * Connecting incompatible gates raises an error during initialize. The gate
     * parameter must be a valid gate of this module. The gate should be marked
     * with @labels(pull) in the NED file.
     */
    virtual bool supportsPacketPulling(cGate *gate) const = 0;

    /**
     * Returns true if the processor supports passing packets as a whole at the
     * given gate. A passed packet is handed over from one module to another
     * by either the standard OMNeT++ handleMessage() mechanism, or by directly
     * calling pushPacket() or pullPacket().
     *
     * For example, packets are passed as a whole to and from a queue module.
     *
     * Connecting incompatible gates raises an error during initialize. The gate
     * parameter must be a valid gate of this module. The gate should be marked
     * with @labels(pass) in the NED file.
     */
    virtual bool supportsPacketPassing(cGate *gate) const = 0;

    /**
     * Returns true if the processor supports streaming packets at the given gate.
     * A streamed packet is handed over from one module to another using several
     * method calls and potentially exending to a non-zero simulation duration.
     *
     * For example, packets are streamed to a preemptable signal transmitter module.
     *
     * Connecting incompatible gates raises an error during initialize. The gate
     * parameter must be a valid gate of this module. The gate should be marked
     * with @labels(stream) in the NED file.
     */
    virtual bool supportsPacketStreaming(cGate *gate) const = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IPACKETPROCESSOR_H

