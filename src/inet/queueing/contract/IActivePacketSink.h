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

#ifndef __INET_IACTIVEPACKETSINK_H
#define __INET_IACTIVEPACKETSINK_H

#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for active packet sinks.
 * See the corresponding NED file for more details.
 */
class INET_API IActivePacketSink
{
  public:
    virtual ~IActivePacketSink() {}

    /**
     * Returns the passive packet source from where packets are pulled or nullptr
     * if the connected module doesn't implement the interface.
     *
     * The gate parameter must be a valid gate of this module.
     */
    virtual IPassivePacketSource *getProvider(cGate *gate) = 0;

    /**
     * Notifies about a state change that allows to pull some packet from the
     * passive packet source at the given gate.
     *
     * This method is called, for example, when a new packet is inserted into
     * a queue. It allows the sink to pull a new packet from the queue.
     *
     * The gate parameter must be a valid gate of this module.
     */
    virtual void handleCanPullPacket(cGate *gate) = 0;

    /**
     * Notifies about the completion of the packet processing for a packet that
     * was pulled earlier independently whether the packet is passed or streamed.
     *
     * This method is called, for example, when a previously pulled packet is
     * failed to be processed successfully. It allows the sink to retry the
     * operation.
     *
     * The gate parameter must be a valid gate of this module. The packet must
     * not be nullptr.
     */
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IACTIVEPACKETSINK_H

