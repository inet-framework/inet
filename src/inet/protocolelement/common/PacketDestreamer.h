//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETDESTREAMER_H
#define __INET_PACKETDESTREAMER_H

#include "inet/common/ModuleRefByGate.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPacketFlow.h"

namespace inet {

using namespace inet::queueing;

class INET_API PacketDestreamer : public PacketProcessorBase, public virtual IPacketFlow
{
  protected:
    bps datarate;

    cGate *inputGate = nullptr;
    ModuleRefByGate<IActivePacketSource> producer;
    ModuleRefByGate<IPassivePacketSource> provider;

    cGate *outputGate = nullptr;
    ModuleRefByGate<IPassivePacketSink> consumer;
    ModuleRefByGate<IActivePacketSink> collector;

    bps streamDatarate = bps(NaN);
    Packet *streamedPacket = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual bool isStreaming() const { return streamedPacket != nullptr; }

  public:
    virtual ~PacketDestreamer();

    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return this; }
    virtual IPassivePacketSource *getProvider(const cGate *gate) override { return this; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPassing(const cGate *gate) const override { return gate == outputGate; }
    virtual bool supportsPacketStreaming(const cGate *gate) const override { return gate == inputGate; }

    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;
    virtual void pushPacket(Packet *packet, const cGate *gate) override { throw cRuntimeError("Invalid operation"); }

    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;

    virtual bool canPullSomePacket(const cGate *gate) const override;
    virtual Packet *canPullPacket(const cGate *gate) const override;
    virtual Packet *pullPacket(const cGate *gate) override;

    virtual Packet *pullPacketStart(const cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketEnd(const cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength) override { throw cRuntimeError("Invalid operation"); }

    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
    virtual void handleCanPullPacketChanged(const cGate *gate) override;
};

} // namespace inet

#endif

