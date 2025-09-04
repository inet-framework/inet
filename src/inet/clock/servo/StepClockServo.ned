//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.servo;

import inet.clock.base.ClockServoBase;
import inet.clock.contract.IClockServo;

//
// Step (bang-bang) clock servo.
//
// Summary
// -------
// Implements ~IClockServo by applying measurements as immediate corrections:
// - Offset is corrected by stepping the clock time (instant jump).
// - Frequency error is corrected by immediately updating the oscillator
//   compensation/rate.
// This may introduce discontinuities (including backward steps) in the clock
// time and can affect modules that assume monotonic clock progression.
//
// Semantics
// ---------
// On each adjust call with (timeDifference Δc, rateDifference Δf_ppm):
// - If Δc > 0: local clock is ahead → step the clock backward by Δc.
// - If Δc < 0: local clock is behind → step the clock forward by |Δc|.
// - Apply a new frequency compensation that cancels Δf_ppm immediately
//   (Δf_ppm > 0 means local runs faster; set compensation to slow it by Δf_ppm).
//
// Effects
// -------
// - Absolute “at” events scheduled on the clock will shift with the time step.
// - Relative “after” events are insensitive to the offset step but will reflect
//   the new rate going forward.
// - Conversions at the step instant follow IClock’s lower/upper-bound conventions.
//
// Parameters
// ----------
// - clockModule (inherited from ~ClockServoBase): path to the controlled clock.
//
// Notes
// -----
// - Use only where abrupt corrections are acceptable (e.g., test, cold-start).
// - For smoother behavior, use a slewing/PI/PLL-style servo instead.
//
// @see ~inet.clock.base.ClockServoBase, ~inet.clock.contract.IClockServo, ~inet.clock.contract.IClock
//
module StepClockServo extends ClockServoBase like IClockServo
{
    parameters:
        @class(StepClockServo);
}
