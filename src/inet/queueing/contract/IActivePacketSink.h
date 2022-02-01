//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
     * Notifies about a change in the possibility of pulling some packet from
     * the passive packet source at the given gate.
     *
     * This method is called, for example, when a new packet is inserted into
     * a queue. It allows the sink to pull a new packet from the queue.
     *
     * The gate parameter must be a valid gate of this module.
     */
    virtual void handleCanPullPacketChanged(cGate *gate) = 0;

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

#endif

