//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CLOCKBASE_H
#define __INET_CLOCKBASE_H

#include "inet/clock/common/ClockTime.h"
#include "inet/clock/common/ClockEvent.h"
#include "inet/clock/contract/IClock.h"
#include "inet/common/DebugDefs.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/SimpleModule.h"

namespace inet {

class INET_API ClockBase : public SimpleModule, public IClock
{
  public:
    static simsignal_t timeChangedSignal;

  protected:
    ModuleRefByPar<IClock> referenceClockModule;
    simtime_t emitClockTimeInterval;
    cMessage *timer = nullptr;
    mutable clocktime_t lastClockTime;

  protected:
    virtual ~ClockBase() { cancelAndDelete(timer); }
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    virtual void checkScheduledClockEvent(const ClockEvent *event) const;

    cSimpleModule* getTargetModule() const;

    virtual void scheduleTargetModuleClockEventAt(simtime_t time, ClockEvent *event);
    virtual void scheduleTargetModuleClockEventAfter(simtime_t time, ClockEvent *event);
    virtual ClockEvent *cancelTargetModuleClockEvent(ClockEvent *event);

    virtual simtime_t computeScheduleTime(clocktime_t time) const;

  public:
    virtual clocktime_t getClockTime() const override;

    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t time, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;
    virtual bool isScheduledClockEvent(ClockEvent *event) const override { return event->isScheduled(); }

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif

