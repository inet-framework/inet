//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETCLONER_H
#define __INET_PACKETCLONER_H

#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API PacketCloner : public PacketProcessorBase, public virtual IPassivePacketSink, public virtual IActivePacketSource
{
  protected:
    cGate *inputGate = nullptr;
    IActivePacketSource *producer = nullptr;

    std::vector<cGate *> outputGates;
    std::vector<IPassivePacketSink *> consumers;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual bool supportsPacketPushing(cGate *gate) const override { return inputGate == gate; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return consumers[gate->getIndex()]; }

    virtual bool canPushSomePacket(cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return true; }

    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

