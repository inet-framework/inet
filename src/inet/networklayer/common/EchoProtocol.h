//
// Copyright (C) 2004, 2009 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ECHOPROTOCOL_H
#define __INET_ECHOPROTOCOL_H

#include "inet/common/SimpleModule.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/EchoPacket_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"

namespace inet {

using namespace inet::queueing;

/**
 * TODO
 */
class INET_API EchoProtocol : public SimpleModule, public IPassivePacketSink
{
  protected:
    PassivePacketSinkRef ipOutSink;

  protected:
    virtual void processPacket(Packet *packet);
    virtual void processEchoRequest(Packet *packet);
    virtual void processEchoReply(Packet *packet);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

  public:
    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }
};

} // namespace inet

#endif

