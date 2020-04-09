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

#ifndef __INET_IACTIVEPACKETSOURCE_H
#define __INET_IACTIVEPACKETSOURCE_H

#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for active packet sources.
 * See the corresponding NED file for more details.
 */
class INET_API IActivePacketSource
{
  public:
    virtual ~IActivePacketSource() {}

    /**
     * Returns the passive packet sink where packets are pushed or nullptr if
     * the connected module doesn't implement the interface.
     *
     * The gate parameter must be a valid gate of this module.
     */
    virtual IPassivePacketSink *getConsumer(cGate *gate) = 0;

    /**
     * Notifies about a state change that allows to push some packet into the
     * passive packet sink at the given gate.
     *
     * This method is called, for example, when a new packet can be inserted
     * into a queue. It allows the source to push a new packet into the queue.
     *
     * The gate parameter must be a valid gate of this module.
     */
    virtual void handleCanPushPacket(cGate *gate) = 0;

    /**
     * Notifies about the completion of the packet processing for a packet that
     * was pushed earlier independently whether the packet is passed or streamed.
     *
     * This method is called, for example, when a previously pushed packet is
     * failed to be processed successfully. It allows the source to retry the
     * operation.
     *
     * The gate parameter must be a valid gate of this module. The packet must
     * not be nullptr.
     */
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) = 0;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_IACTIVEPACKETSOURCE_H

