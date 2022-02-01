//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPSPOOF_H
#define __INET_TCPSPOOF_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {
namespace tcp {

/**
 * Sends fabricated TCP packets.
 */
class INET_API TcpSpoof : public cSimpleModule
{
  protected:
    virtual void sendToIP(Packet *pk, L3Address src, L3Address dest);
    virtual unsigned long chooseInitialSeqNum();
    virtual void sendSpoofPacket();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace tcp
} // namespace inet

#endif

