//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETCLONER_H
#define __INET_PACKETCLONER_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API PacketCloner : public PacketProcessorBase, public virtual IPassivePacketSink, public virtual IActivePacketSource
{
  protected:
    cGate *inputGate = nullptr;
    ModuleRefByGate<IActivePacketSource> producer;

    std::vector<cGate *> outputGates;
    std::vector<ModuleRefByGate<IPassivePacketSink>> consumers;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

  public:
    virtual bool supportsPacketPushing(const cGate *gate) const override { return inputGate == gate; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }

    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return consumers[gate->getIndex()]; }

    virtual bool canPushSomePacket(const cGate *gate) const override { return true; }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return true; }

    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

