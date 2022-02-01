//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSCILLATORBASEDCLOCK_H
#define __INET_OSCILLATORBASEDCLOCK_H

#include "inet/clock/base/ClockBase.h"
#include "inet/clock/contract/IOscillator.h"

namespace inet {

class INET_API OscillatorBasedClock : public ClockBase, public cListener
{
  protected:
    IOscillator *oscillator = nullptr;
    int64_t (*roundingFunction)(int64_t, int64_t) = nullptr;

    int64_t originClockTick = -1; // the value of the clock internal register representing the time as known by the clock measured in oscillator nominal tick intervals
    std::vector<ClockEvent *> events;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual ~OscillatorBasedClock();

    virtual clocktime_t computeClockTimeFromSimTime(simtime_t t) const override;
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t t) const override;

    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;

    virtual const char *resolveDirective(char directive) const override;

    virtual void receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

