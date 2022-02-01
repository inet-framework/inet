//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETMETERBASE_H
#define __INET_PACKETMETERBASE_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {

using namespace inet::queueing;

class INET_API PacketMeterBase : public PacketProcessorBase, public IPassivePacketSink, public IActivePacketSource
{
  protected:
    cGate *inputGate = nullptr;
    IActivePacketSource *producer = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual int meterPacket(Packet *packet) = 0;

  public:
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { throw cRuntimeError("Invalid operation"); }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return true; }

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }
};

} // namespace inet

#endif

