//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PREEMPTABLESTREAMER_H
#define __INET_PREEMPTABLESTREAMER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/common/ActivePacketSinkRef.h"
#include "inet/queueing/common/ActivePacketSourceRef.h"
#include "inet/queueing/common/PassivePacketSinkRef.h"
#include "inet/queueing/common/PassivePacketSourceRef.h"
#include "inet/queueing/contract/IPacketFlow.h"

namespace inet {

using namespace inet::queueing;

extern template class ClockUserModuleMixin<PacketProcessorBase>;

class INET_API PreemptableStreamer : public ClockUserModuleMixin<PacketProcessorBase>, public virtual IPacketFlow
{
  protected:
    bps datarate = bps(NaN);
    b minPacketLength = b(-1);
    b roundingLength = b(-1);

    cGate *inputGate = nullptr;
    ActivePacketSourceRef producer;
    PassivePacketSourceRef provider;

    cGate *outputGate = nullptr;
    PassivePacketSinkRef consumer;
    ActivePacketSinkRef collector;

    simtime_t streamStart;
    bps streamDatarate = bps(NaN);
    Packet *streamedPacket = nullptr;
    Packet *remainingPacket = nullptr;

    ClockEvent *endStreamingTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual bool isStreaming() const { return streamedPacket != nullptr; }
    virtual void endStreaming();

  public:
    virtual ~PreemptableStreamer();

    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return this; }
    virtual IPassivePacketSource *getProvider(const cGate *gate) override { return this; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPassing(const cGate *gate) const override { return gate == inputGate; }
    virtual bool supportsPacketStreaming(const cGate *gate) const override { return gate == outputGate; }

    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;
    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override;
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;

    virtual bool canPullSomePacket(const cGate *gate) const override;
    virtual Packet *canPullPacket(const cGate *gate) const override;
    virtual Packet *pullPacket(const cGate *gate) override { throw cRuntimeError("Invalid operation"); }

    virtual Packet *pullPacketStart(const cGate *gate, bps datarate) override;
    virtual Packet *pullPacketEnd(const cGate *gate) override;
    virtual Packet *pullPacketProgress(const cGate *gate, bps datarate, b position, b extraProcessableLength) override;

    virtual void handleCanPullPacketChanged(const cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;
};

} // namespace inet

#endif

