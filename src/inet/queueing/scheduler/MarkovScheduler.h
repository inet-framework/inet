//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MARKOVSCHEDULER_H
#define __INET_MARKOVSCHEDULER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketSchedulerBase.h"
#include "inet/queueing/common/ActivePacketSourceRef.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PacketSchedulerBase>;

namespace queueing {

class INET_API MarkovScheduler : public ClockUserModuleMixin<PacketSchedulerBase>, public virtual IPassivePacketSink, public virtual IActivePacketSource
{
  protected:
    std::vector<ActivePacketSourceRef> producers;
    PassivePacketSinkRef consumer;

    std::vector<std::vector<double>> transitionProbabilities;
    std::vector<cDynamicExpression> waitIntervals;

    int state;

    ClockEvent *waitTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual int schedulePacket() override;
    virtual void scheduleWaitTimer();

  public:
    virtual ~MarkovScheduler();

    virtual IPassivePacketSink *getConsumer(const cGate *gate) override { return consumer; }

    virtual bool supportsPacketPushing(const cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(const cGate *gate) const override { return true; }

    virtual bool canPushSomePacket(const cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override;

    virtual void pushPacket(Packet *packet, const cGate *gate) override;

    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("Invalid operation"); }

    virtual void handleCanPushPacketChanged(const cGate *gate) override;
    virtual void handlePushPacketProcessed(Packet *packet, const cGate *gate, bool successful) override;

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif

