//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECEIVEFROMMACADDRESS_H
#define __INET_RECEIVEFROMMACADDRESS_H

#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * Pops a SourceMacAddressHeader and records the sender's MAC address into a MacAddressInd tag,
 * so a downstream ReceiveWithAcknowledge can address its acknowledgement back to the sender.
 * The counterpart of ReceiveAtMacAddress; never drops the packet.
 */
class INET_API ReceiveFromMacAddress : public PacketFlowBase
{
  protected:
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif
