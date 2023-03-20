//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PERIODICGATE_H
#define __INET_PERIODICGATE_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketGateBase.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PacketGateBase>;

namespace queueing {

class INET_API PeriodicGate : public ClockUserModuleMixin<PacketGateBase>
{
  public:
    static simsignal_t guardBandStateChangedSignal;

  protected:
    int index = 0;
    bool initiallyOpen = false;
    clocktime_t initialOffset;
    clocktime_t offset;
    clocktime_t totalDuration = 0;
    std::vector<clocktime_t> durations;
    bool scheduleForAbsoluteTime = false;
    bool enableImplicitGuardBand = true;
    int openSchedulingPriority = 0;
    int closeSchedulingPriority = 0;
    bool isInGuardBand_ = false;

    ClockEvent *changeTimer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *message) override;
    virtual void handleParameterChange(const char *name) override;
    virtual bool canPacketFlowThrough(Packet *packet) const override;

    virtual void initializeGating();
    virtual void scheduleChangeTimer();
    virtual void processChangeTimer();
    virtual void readDurationsPar();

    virtual void updateIsInGuardBand();

  public:
    virtual ~PeriodicGate() { cancelAndDelete(changeTimer); }

    virtual clocktime_t getInitialOffset() const { return initialOffset; }
    virtual bool getInitiallyOpen() const { return initiallyOpen; }
    virtual const std::vector<clocktime_t>& getDurations() const { return durations; }

    virtual bool isInGuardBand() const { return isInGuardBand_; }

    virtual void open() override;
    virtual void close() override;

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;
};

} // namespace queueing
} // namespace inet

#endif

