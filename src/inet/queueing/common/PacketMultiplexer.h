//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETMULTIPLEXER_H
#define __INET_PACKETMULTIPLEXER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API PacketMultiplexer : public PacketProcessorBase, public virtual IPassivePacketSink, public virtual IActivePacketSource, public TransparentProtocolRegistrationListener
{
  protected:
    bool forwardServiceRegistration;
    bool forwardProtocolRegistration;

    std::vector<cGate *> inputGates;
    std::vector<IActivePacketSource *> producers;

    cGate *outputGate = nullptr;
    IPassivePacketSink *consumer = nullptr;

    int inProgressStreamId = -1;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void mapRegistrationForwardingGates(cGate *gate, std::function<void(cGate *)> f) override;

    virtual bool isStreamingPacket() const { return inProgressStreamId != -1; }
    virtual void startPacketStreaming(Packet *packet);
    virtual void endPacketStreaming(Packet *packet);
    virtual void checkPacketStreaming(Packet *packet);

  public:
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return consumer; }

    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }
    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(cGate *gate) const override { return consumer->canPushSomePacket(outputGate); }
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override { return consumer->canPushPacket(packet, gate); }

    virtual void pushPacket(Packet *packet, cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override;
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override;

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual bool isForwardingService(cGate *gate, ServicePrimitive servicePrimitive) const override { return forwardServiceRegistration; }
    virtual bool isForwardingProtocol(cGate *gate, ServicePrimitive servicePrimitive) const override { return forwardProtocolRegistration; }
};

} // namespace queueing
} // namespace inet

#endif

