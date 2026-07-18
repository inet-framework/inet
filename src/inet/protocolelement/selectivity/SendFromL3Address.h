//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SENDFROML3ADDRESS_H
#define __INET_SENDFROML3ADDRESS_H

#include "inet/networklayer/common/L3Address.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * Prepends a SourceL3AddressHeader carrying this node's L3 address, so the receiver
 * (ReceiveFromL3Address) can learn who sent the packet. The counterpart of SendToL3Address.
 */
class INET_API SendFromL3Address : public PacketFlowBase
{
  protected:
    L3Address address;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

