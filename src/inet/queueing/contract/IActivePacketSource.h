//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
     * Notifies about a change in the possibility of pushing some packet into
     * the passive packet sink at the given gate.
     *
     * This method is called, for example, when a new packet can be inserted
     * into a queue. It allows the source to push a new packet into the queue.
     *
     * The gate parameter must be a valid gate of this module.
     */
    virtual void handleCanPushPacketChanged(cGate *gate) = 0;

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

#endif

