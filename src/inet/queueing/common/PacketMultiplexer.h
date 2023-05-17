//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETMULTIPLEXER_H
#define __INET_PACKETMULTIPLEXER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/common/ActivePacketSourceRef.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {
namespace queueing {

class INET_API PacketMultiplexer : public PacketProcessorBase, public virtual IPassivePacketSink, public virtual IActivePacketSource, public TransparentProtocolRegistrationListener
{
  protected:
    bool forwardServiceRegistration;
    bool forwardProtocolRegistration;

    std::vector<cGate *> inputGates;
    std::vector<ActivePacketSourceRef> producers;

    cGate *outputGate = nullptr;
    PassivePacketSinkRef consumer;

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
    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return consumer; }

    virtual bool supportsPacketPulling(const cGate *gate) const override { return false; }
    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(const cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(const cGate *gate) const override { return consumer.canPushSomePacket(); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return consumer.canPushPacket(packet); }

    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override;
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override;
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override;

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;

    virtual bool isForwardingService(cGate *gate, ServicePrimitive servicePrimitive) const override { return forwardServiceRegistration; }
    virtual bool isForwardingProtocol(cGate *gate, ServicePrimitive servicePrimitive) const override { return forwardProtocolRegistration; }
};

} // namespace queueing
} // namespace inet

#endif

