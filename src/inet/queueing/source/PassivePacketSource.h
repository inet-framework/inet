//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PASSIVEPACKETSOURCE_H
#define __INET_PASSIVEPACKETSOURCE_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PassivePacketSourceBase.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PassivePacketSourceBase>;

namespace queueing {

class INET_API PassivePacketSource : public ClockUserModuleMixin<PassivePacketSourceBase>
{
  protected:
    clocktime_t initialProvidingOffset;
    cPar *providingIntervalParameter = nullptr;
    ClockEvent *providingTimer = nullptr;
    bool scheduleForAbsoluteTime = false;

    mutable Packet *nextPacket = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void scheduleProvidingTimer(clocktime_t delay);
    virtual Packet *providePacket(cGate *gate);

  public:
    virtual ~PassivePacketSource() { delete nextPacket; cancelAndDeleteClockEvent(providingTimer); }

    virtual bool supportsPacketPushing(cGate *gate) const override { return false; }
    virtual bool supportsPacketPulling(cGate *gate) const override { return outputGate == gate; }

    virtual bool canPullSomePacket(cGate *gate) const override;
    virtual Packet *canPullPacket(cGate *gate) const override;

    virtual Packet *pullPacket(cGate *gate) override;
    virtual Packet *pullPacketStart(cGate *gate, bps datarate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketEnd(cGate *gate) override { throw cRuntimeError("Invalid operation"); }
    virtual Packet *pullPacketProgress(cGate *gate, bps datarate, b position, b extraProcessableLength) override { throw cRuntimeError("Invalid operation"); }
};

} // namespace queueing
} // namespace inet

#endif

