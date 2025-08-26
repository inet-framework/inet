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
  public:
    static simsignal_t oscillatorCompensationChangedSignal;

  protected:
    /** Default policy for handling overdue events after a clock step. */
    OverdueClockEventHandlingMode defaultOverdueClockEventHandlingMode = UNSPECIFIED;

    /*
     * Intuition for compensation vs. the internal counter:
     * If the nominal tick length is 1 µs (with 1 ps precision), a pure nominal
     * clock would add +1'000'000 “subticks” per tick. With +100 ppm
     * compensation, the effective per-tick increment alternates so that the
     * *average* is +1'000'100, realized through x and the fractional accumulator
     * p. The reported clock time is the accumulated (effective) ticks times the
     * nominal tick length.
     */

    /**
     * Current oscillator compensation in ppm:
     *   0 ppm  ⇒ no compensation (x ≈ 1)
     *  >0 ppm  ⇒ faster clock (x > 1; shorter effective tick)
     *  <0 ppm  ⇒ slower clock (x < 1; longer effective tick)
     *
     * Note: “faster/slower” refers to measured *clock time per simulation time*
     * as produced by the compensation layer, not the physical oscillator.
     */
    ppm oscillatorCompensation = ppm(0);

  protected:
    virtual void initialize(int stage) override;

    virtual OverdueClockEventHandlingMode getOverdueClockEventHandlingMode(ClockEvent *event) const;

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  public:
    virtual IClock *getUnderlyingClock() { return check_and_cast<IClock *>(getSubmodule("underlyingClock")); }

    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;

    virtual SimTimeScale getOscillatorCompensation() const override {
        return SimTimeScale::fromPpm(oscillatorCompensation.get<ppm>());
    }

    /**
     * Sets the clock time to the given value. Scheduled overdue clock events
     * can be executed synchronously before return.
     * Greater than 1 oscillator compensation factor means the clock measures time faster.
     */
    virtual void setClockTime(clocktime_t time, ppm oscillatorCompensation, bool resetOscillator);
};

} // namespace inet

#endif
