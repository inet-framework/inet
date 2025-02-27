//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.contract;

//
// Interface for clock servo models.
//
// Summary
// -------
// A clock servo is a control component that disciplines a local clock
// (implementing ~IClock) against some external or reference clock.
// It consumes measurements of offset (time difference) and frequency
// error (rate difference) between the local clock and the reference,
// and issues adjustments to the controlled clock.
//
// Semantics
// ---------
// - Input: a measured timeDifference Δc and rateDifference Δf.
//     * Δc > 0 ⇒ local clock is ahead of the reference.
//     * Δf > 0 ⇒ local clock runs faster than the reference (in ppm).
// - Output: adjustments applied to the associated ~IClock, such as
//   step corrections, slewing, or drift updates.
// - The servo defines the control law: e.g., proportional, PI, PLL,
//   FLL, or more complex algorithms.
// - A servo may smooth noisy inputs, filter jitter, and enforce limits
//   on step size or rate correction.
//
// Typical use
// -----------
// - Instantiate a servo module that implements this interface.
// - Connect it logically to a clock module (implementing ~IClock).
// - Drive the servo with periodic or event-based measurements of clock
//   differences, typically produced by a synchronization protocol
//   (e.g., IEEE 1588 / gPTP, NTP, or custom).
// - The servo translates those measurements into disciplined behavior
//   of the local clock.
//
// Notes
// -----
// - This interface only defines the contract; specific servo algorithms
//   (e.g., linear, PI, adaptive) must be implemented by subclasses.
// - Servos should preserve the documented invariants of ~IClock
//   (e.g., monotonicity except at explicit steps).
//
// @see ~IClock, ~IOscillator
//
moduleinterface IClockServo
{
    parameters:
        @display("i=block/control");
}
