//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECEIVEFROML3ADDRESS_H
#define __INET_RECEIVEFROML3ADDRESS_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * Pops a SourceL3AddressHeader and records the sender's address into an L3AddressInd tag
 * for higher elements. Unlike a destination check, this never drops the packet; it is a
 * plain flow, not a filter. The counterpart of ReceiveAtL3Address.
 */
class INET_API ReceiveFromL3Address : public PacketFlowBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

