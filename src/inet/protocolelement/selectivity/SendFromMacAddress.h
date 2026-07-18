//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SENDFROMMACADDRESS_H
#define __INET_SENDFROMMACADDRESS_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

/**
 * Prepends a SourceMacAddressHeader carrying this interface's MAC address, so the receiver
 * (ReceiveFromMacAddress) learns who sent the frame -- which lets ReceiveWithAcknowledge unicast
 * the acknowledgement back to the sender on a shared medium. The source-MAC counterpart of
 * SendToMacAddress.
 */
class INET_API SendFromMacAddress : public PacketFlowBase
{
  protected:
    MacAddress address;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif
