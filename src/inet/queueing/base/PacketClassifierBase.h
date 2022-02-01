//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETCLASSIFIERBASE_H
#define __INET_PACKETCLASSIFIERBASE_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPacketSink.h"
#include "inet/queueing/contract/IPacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketClassifierBase : public PacketProcessorBase, public TransparentProtocolRegistrationListener, public virtual IPacketSink, public virtual IPacketSource
{
  protected:
    bool reverseOrder = false;

    cGate *inputGate = nullptr;
    IActivePacketSource *producer = nullptr;
    IPassivePacketSource *provider = nullptr;

    std::vector<cGate *> outputGates;
    std::vector<IPassivePacketSink *> consumers;
    std::vector<IActivePacketSink *> collectors;

    int inProgressStreamId = -1;
    int inProgressGateIndex = -1;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void mapRegistrationForwardingGates(cGate *gate, std::function<void(cGate *)> f) override;

    virtual size_t getOutputGateIndex(size_t i) const { return reverseOrder ? outputGates.size() - i - 1 : i; }
    virtual int classifyPacket(Packet *packet) = 0;
    virtual int callClassifyPacket(Packet *packet) const;

    virtual bool isStreamingPacket() const { return inProgressStreamId != -1; }
    virtual void startPacketStreaming(Packet *packet);
    virtual void endPacketStreaming(Packet *packet);
    virtual void checkPacketStreaming(Packet *packet);

  public:
    virtual IPassivePacketSource *getProvider(cGate *gate) override { return provider; }
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return consumers[gate->getIndex()]; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;

    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual bool canPullSomePacket(cGate *gate) const override;
    virtual Packet *canPullPacket(cGate *gate) const override;

    virtual Packet *pullPacket(cGate *gate) override;
    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual Packet *pullPacketEnd(cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) override { throw cRuntimeError("TODO"); }

    virtual void handleCanPullPacketChanged(cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override { throw cRuntimeError("TODO"); }
};

} // namespace queueing
} // namespace inet

#endif

