//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PASSIVEPACKETSINK_H
#define __INET_PASSIVEPACKETSINK_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PassivePacketSinkBase.h"
#include "inet/queueing/contract/IActivePacketSource.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PassivePacketSinkBase>;

namespace queueing {

class INET_API PassivePacketSink : public ClockUserModuleMixin<PassivePacketSinkBase>
{
  protected:
    clocktime_t initialConsumptionOffset;
    cPar *consumptionIntervalParameter = nullptr;
    ClockEvent *consumptionTimer = nullptr;
    bool scheduleForAbsoluteTime = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleConsumptionTimer(clocktime_t delay);
    virtual void consumePacket(Packet *packet);

  public:
    virtual ~PassivePacketSink() { cancelAndDeleteClockEvent(consumptionTimer); }

    virtual bool supportsPacketPushing(cGate *gate) const override { return gate == inputGate; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return false; }

    virtual bool canPushSomePacket(cGate *gate) const override;
    virtual bool canPushPacket(Packet *packet, cGate *gate) const override;

    virtual void pushPacket(Packet *packet, cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

