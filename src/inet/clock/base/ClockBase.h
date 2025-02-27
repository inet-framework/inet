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
#include "inet/common/StringFormat.h"
#include "inet/common/ModuleRefByPar.h"

namespace inet {

class INET_API ClockBase : public cSimpleModule, public IClock, public StringFormat::IDirectiveResolver
{
  public:
    static simsignal_t timeChangedSignal;

  protected:
    const char *displayStringTextFormat = nullptr;
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
    virtual void refreshDisplay() const override;

    cSimpleModule *getTargetModule() const {
        cSimpleModule *target = getSimulation()->getContextSimpleModule();
        if (target == nullptr)
            throw cRuntimeError("scheduleAt()/cancelEvent() must be called with a simple module in context");
        return target;
    }

    virtual void scheduleTargetModuleClockEventAt(simtime_t time, ClockEvent *event);
    virtual void scheduleTargetModuleClockEventAfter(simtime_t time, ClockEvent *event);
    virtual ClockEvent *cancelTargetModuleClockEvent(ClockEvent *event);

    simtime_t computeScheduleTime(clocktime_t time);

    void checkClockEvent(const ClockEvent *event) {
        ASSERT(event->isScheduled());
        ASSERTCMP(>=, event->getArrivalTime(), simTime());
        // NOTE: IClock interface 3. invariant
        ASSERTCMP(>=, event->getArrivalClockTime(), getClockTime());
        // NOTE: IClock interface 4. invariant
        ASSERTCMP(==, event->getArrivalTime(), computeScheduleTime(event->getArrivalClockTime()));
        // NOTE: IClock interface 5. invariant
        ASSERTCMP(==, event->getArrivalClockTime(), computeClockTimeFromSimTime(event->getArrivalTime()));
    }

  public:
    virtual clocktime_t getClockTime() const override;

    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t time, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace inet

#endif

