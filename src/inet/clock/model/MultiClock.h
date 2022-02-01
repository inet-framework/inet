//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MULTICLOCK_H
#define __INET_MULTICLOCK_H

#include "inet/clock/common/ClockEvent.h"
#include "inet/clock/common/ClockTime.h"
#include "inet/clock/contract/IClock.h"

namespace inet {

class INET_API MultiClock : public cModule, public virtual IClock, public cListener
{
  protected:
    IClock *activeClock = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

  public:
    virtual clocktime_t getClockTime() const override { return activeClock->getClockTime(); }
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t time) const override { return activeClock->computeClockTimeFromSimTime(time); }
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t time) const override { return activeClock->computeSimTimeFromClockTime(time); }
    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *event) override { activeClock->scheduleClockEventAt(time, event); }
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override { activeClock->scheduleClockEventAfter(delay, event); }
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override { return activeClock->cancelClockEvent(event); }
    virtual void handleClockEvent(ClockEvent *event) override { activeClock->handleClockEvent(event); }

    virtual void receiveSignal(cComponent *source, int signal, const simtime_t& time, cObject *details) override;
};

} // namespace inet

#endif

