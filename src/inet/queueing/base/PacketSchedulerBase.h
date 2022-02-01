//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETSCHEDULERBASE_H
#define __INET_PACKETSCHEDULERBASE_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/contract/IPacketSink.h"
#include "inet/queueing/contract/IPacketSource.h"

namespace inet {
namespace queueing {

class INET_API PacketSchedulerBase : public PacketProcessorBase, public TransparentProtocolRegistrationListener, public virtual IPacketSink, public virtual IPacketSource
{
  protected:
    bool reverseOrder = false;

    std::vector<cGate *> inputGates;
    std::vector<IPassivePacketSource *> providers;
    std::vector<IActivePacketSource *> producers;

    cGate *outputGate = nullptr;
    IActivePacketSink *collector = nullptr;
    IPassivePacketSink *consumer = nullptr;

    int inProgressStreamId = -1;
    int inProgressGateIndex = -1;

  protected:
    virtual void initialize(int stage) override;

    virtual void mapRegistrationForwardingGates(cGate *gate, std::function<void(cGate *)> f) override;

    virtual size_t getInputGateIndex(size_t i) const { return reverseOrder ? inputGates.size() - i - 1 : i; }
    virtual int schedulePacket() = 0;
    virtual int callSchedulePacket() const;

    virtual bool isStreamingPacket() const { return inProgressStreamId != -1; }
    virtual void startPacketStreaming();
    virtual void endPacketStreaming(Packet *packet);
    virtual void checkPacketStreaming(Packet *packet);

  public:
    virtual IPassivePacketSink *getConsumer(cGate *gate) override { return consumer; }
    virtual IPassivePacketSource *getProvider(cGate *gate) override { return providers[gate->getIndex()]; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return true; }
    virtual bool supportsPacketStreaming(cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;

    virtual void pushPacket(Packet *packet, cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, cGate *gate, bool successful) override { throw cRuntimeError("TODO"); }

    virtual bool canPullSomePacket(cGate *gate) const override;
    virtual Packet *canPullPacket(cGate *gate) const override;

    virtual Packet *pullPacket(cGate *gate) override;
    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override;
    virtual Packet *pullPacketEnd(cGate *gate) override;
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) override;

    virtual void handleCanPullPacketChanged(cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;
};

} // namespace queueing
} // namespace inet

#endif

