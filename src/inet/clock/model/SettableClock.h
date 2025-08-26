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
 * @brief Clock that can be stepped (set) and whose effective oscillator
 * compensation can be changed at runtime.
 *
 * Overview
 * --------
 * SettableClock extends ~OscillatorBasedClock with two capabilities:
 *  1) **Stepping** the clock time to an explicitly provided value (which may
 *     jump forward or backward), and
 *  2) **Changing the oscillator-compensation factor** used by the clock’s
 *     per-tick compensation step function (see OscillatorBasedClock).
 *
 * Stepping creates a discontinuity in C(t) (the simulation→clock mapping).
 * Conversions at the step instant follow the lower/upper-bound conventions
 * documented by ~IClock. When stepping forward, previously scheduled events
 * might become overdue; behavior is controlled by
 * OverdueClockEventHandlingMode (see ClockBase).
 *
 * Oscillator compensation
 * -----------------------
 * The compensation is expressed in parts per million:
 *   - `oscillatorCompensation` = r_ppm
 *   - Effective factor x ≈ 1 + r_ppm·1e−6  (x > 1 ⇒ faster clock)
 *
 * This x is the “near-1” scale factor used by OscillatorBasedClock’s formulas:
 *   A(n) = floor( p + (x − 1)·n ),  F(n) = n + A(n).
 * Internally, the fractional accumulator p (Q0.63) carries the residual of past
 * compensation so that long-term rate matches x with bounded per-tick error.
 *
 * Script integration
 * ------------------
 * As an IScriptable component, the clock can be manipulated from
 * ScenarioManager (e.g., setting time and compensation at scripted instants).
 *
 * Invariants (inherited)
 * ----------------------
 * - cos (clock origin sim time) ≥ oos (oscillator origin)
 * - cos ≤ now (current simulation time)
 * - Conversions are monotone between explicit modifications; steps introduce a
 *   jump that is handled via lower/upper bound selection.
 */
class INET_API SettableClock : public OscillatorBasedClock, public IScriptable
{
  public:
    /** Emitted after the oscillator compensation (x) is changed. */
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
    /** Read parameters, set initial compensation, and chain to the base. */
    virtual void initialize(int stage) override;

    /**
     * Returns the policy that determines how to deal with clock events that
     * become overdue due to a clock step (forward or backward). If an event’s
     * target time is now in the past after the step, the policy (see ClockBase)
     * determines whether the event is executed immediately, canceled,
     * rescheduled, or left unchanged.
     */
    virtual OverdueClockEventHandlingMode getOverdueClockEventHandlingMode(ClockEvent *event) const;

    // IScriptable
    /** Parses and applies scenario directives (e.g., “set-clock …”). */
    virtual void processCommand(const cXMLElement& node) override;

  public:
    /**
     * Returns the nested submodule named "underlyingClock" as ~IClock.
     * This allows a composite setup where SettableClock wraps another clock.
     * The submodule must exist and implement ~IClock.
     */
    virtual IClock *getUnderlyingClock() { return check_and_cast<IClock *>(getSubmodule("underlyingClock")); }

    /** @name IClock scheduling API (intercepted to honor step/overdue policies) */
    ///@{
    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;
    virtual void handleClockEvent(ClockEvent *event) override;
    ///@}

    /**
     * Current near-1 compensation factor x as a SimTimeScale (x ≈ 1 + ppm·1e−6).
     * Used by the OscillatorBasedClock formulas A/F and the fractional accumulator.
     */
    virtual SimTimeScale getOscillatorCompensation() const override {
        return SimTimeScale::fromPpm(oscillatorCompensation.get<ppm>());
    }

    /**
     * @brief Step the clock to a new time and (optionally) update compensation.
     *
     * Effects (executed atomically at the current simulation time s = now):
     *  - The clock origin is moved to s, and the new origin clock time is set to `time`
     *    (i.e., setOrigin(c := time) in OscillatorBasedClock terms).
     *  - The oscillator compensation factor is updated to the provided ppm value
     *    (x := 1 + ppm·1e−6). The fractional accumulator p is preserved/updated so
     *    long-term average matches the new x without creating double/missed ticks.
     *  - If `resetOscillator == true`, the phase/origin of the driving oscillator is
     *    also reset so that the first tick not earlier than s is aligned to the new
     *    origin (preserving the no-double/no-missed-tick invariant).
     *  - Pending events affected by the step are handled according to
     *    OverdueClockEventHandlingMode (see getOverdueClockEventHandlingMode()).
     *
     * @param time                 New clock time C(now) after the step.
     * @param oscillatorCompensation
     *                             New compensation in ppm (near 0). Positive speeds up the clock.
     * @param resetOscillator      If true, also reset the underlying oscillator phase/origin.
     *
     * Signals:
     *  - Emits oscillatorCompensationChangedSignal after the compensation update.
     *
     * Notes:
     *  - Backward steps are allowed and may affect modules assuming monotone clock
     *    progression; use the overdue policy to maintain consistency.
     */
    virtual void setClockTime(clocktime_t time, ppm oscillatorCompensation, bool resetOscillator);
};

} // namespace inet

#endif
