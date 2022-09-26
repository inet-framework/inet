//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MARKOVCLASSIFIER_H
#define __INET_MARKOVCLASSIFIER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketClassifierBase.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSource.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PacketClassifierBase>;

namespace queueing {

class INET_API MarkovClassifier : public ClockUserModuleMixin<PacketClassifierBase>, public virtual IActivePacketSink, public virtual IPassivePacketSource
{
  protected:
    IPassivePacketSource *provider = nullptr;
    std::vector<IActivePacketSink *> collectors;

    std::vector<std::vector<double>> transitionProbabilities;
    std::vector<cDynamicExpression> waitIntervals;

    int state;

    ClockEvent *waitTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual int classifyPacket(Packet *packet) override;
    virtual void scheduleWaitTimer();

  public:
    virtual ~MarkovClassifier();

    virtual IPassivePacketSource *getProvider(cGate *gate) override { return provider; }

    virtual bool supportsPacketPushing(cGate *gate) const override { return true; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return true; }

    virtual bool canPullSomePacket(cGate *gate) const override;
    virtual Packet *canPullPacket(cGate *gate) const override;

    virtual Packet *pullPacket(cGate *gate) override;

    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketEnd(cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) override { throw cRuntimeError("Invalid operation"); }

    virtual void handleCanPullPacketChanged(cGate *gate) override;
    virtual void handlePullPacketProcessed(Packet *packet, cGate *gate, bool successful) override;

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif

