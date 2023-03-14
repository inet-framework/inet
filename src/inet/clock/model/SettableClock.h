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
    ppm oscillatorCompensation = ppm(0); // 0 means no compensation, higher value means faster clock, e.g. 100 ppm value means the clock compensates 100 microseconds for every second in clock time
                                         // 100 ppm value means the oscillator tick length is compensated to be smaller by a factor of (1 / (1 + 100 / 1E+6)) than the actual tick length measured in clock time

  protected:
    virtual void initialize(int stage) override;

    virtual OverdueClockEventHandlingMode getOverdueClockEventHandlingMode(ClockEvent *event) const;
    virtual simtime_t handleOverdueClockEvent(ClockEvent *event, simtime_t t);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  public:
    virtual ppm getOscillatorCompensation() const override { return oscillatorCompensation; }

    /**
     * Sets the clock time immediately to the given value. Greater than 1 oscillator
     * compensation factor means the clock measures time faster.
     */
    virtual void setClockTime(clocktime_t time, ppm oscillatorCompensation, bool resetOscillator);
};

} // namespace inet

#endif

