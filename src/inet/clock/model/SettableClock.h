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

/**
 * @brief Clock that can be stepped (set) and whose effective oscillator compensation can be changed at runtime.
 *
 * Stepping creates a discontinuity in C(t) (the simulation time to clock time mapping). Conversions at the step instant
 * follow the lower/upper-bound conventions documented by ~IClock. When stepping forward, previously scheduled events
 * might become overdue; the behavior is controlled by a module parameter.
 */
class INET_API SettableClock : public OscillatorBasedClock, public IScriptable
{
  public:
    static simsignal_t oscillatorCompensationChangedSignal;

  protected:
    OverdueClockEventHandlingMode defaultOverdueClockEventHandlingMode = UNSPECIFIED;
    ppm oscillatorCompensation = ppm(0); // 0 means no compensation, higher value means faster clock, e.g. 100 ppm value means the clock compensates 100 microseconds for every second in clock time
                                         // 100 ppm value means the oscillator tick length is compensated to be smaller by a factor of (1 / (1 + 100 / 1E+6)) than the actual tick length measured in clock time

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
     * @brief Step the clock to a new time and (optionally) update oscillator compensation.
     *
     * Effects (executed atomically at the current simulation time s = now):
     *  - The clock origin is moved to s, and the new origin clock time is set to the given time.
     *  - The oscillator compensation factor is updated to the provided ppm value.
     *  - If resetOscillator is true, the phase/origin of the driving oscillator is also reset.
     *  - Pending events affected by the step are handled according to defaultOverdueClockEventHandlingMode.
     *
     *  Backward steps are allowed and may affect modules assuming monotone clock progression.
     */
    virtual void setClockTime(clocktime_t time, ppm oscillatorCompensation, bool resetOscillator);
};

} // namespace inet

#endif

