//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SETTABLECLOCK_H
#define __INET_SETTABLECLOCK_H

#include "inet/clock/model/OscillatorBasedClock.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

class INET_API SettableClock : public OscillatorBasedClock, public IScriptable
{
  protected:
    OverdueClockEventHandlingMode defaultOverdueClockEventHandlingMode = UNSPECIFIED;

    // TODO explain that the time dilation along the clock step function is the same as if
    // the clock would add a little bit more or less to its internal integer counter than what is required according to the nominal tick length
    // for example, the clock would add 1000000 by default for 1us nominal tick length using 1ps time precision
    // when oscillator compensation is used, the clock adds 1000010 or 999970, etc. instead
    // each time the clock is asked for the clock time, it simply returns the internal counter divided by 1000000

    ppm oscillatorCompensation = ppm(0); // 0 means no compensation, higher value means faster clock, e.g. 100 ppm value means the clock compensates 100 microseconds for every second in clock time
                                         // 100 ppm value means the oscillator tick length is compensated to be smaller by a factor of (1 / (1 + 100 / 1E+6)) than the actual tick length measured in clock time

  protected:
    virtual void initialize(int stage) override;

    virtual OverdueClockEventHandlingMode getOverdueClockEventHandlingMode(ClockEvent *event) const;

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  public:
    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;

    virtual ppm getOscillatorCompensation() const override { return oscillatorCompensation; }

    /**
     * Sets the clock time to the given value. Scheduled overdue clock events
     * can be executed synchronously before return.
     * Greater than 1 oscillator compensation factor means the clock measures time faster.
     */
    virtual void setClockTime(clocktime_t time, ppm oscillatorCompensation, bool resetOscillator);
};

} // namespace inet

#endif

