//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TCPSPOOF_H
#define __INET_TCPSPOOF_H

#include "inet/common/SimpleModule.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {

using namespace inet::queueing;

namespace tcp {

/**
 * Sends fabricated TCP packets.
 */
class INET_API TcpSpoof : public SimpleModule, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef ipOutSink;

    virtual void sendToIP(Packet *pk, L3Address src, L3Address dest);
    virtual unsigned long chooseInitialSeqNum();
    virtual void sendSpoofPacket();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace tcp
} // namespace inet

#endif

