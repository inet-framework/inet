//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECEIVEFROMPORT_H
#define __INET_RECEIVEFROMPORT_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * Pops a SourcePortHeader and records the sender's port into an L4PortInd tag for higher
 * elements. Never drops the packet. The counterpart of ReceiveAtPort.
 */
class INET_API ReceiveFromPort : public PacketFlowBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

