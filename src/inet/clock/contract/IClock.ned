//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.contract;

//
// Interface for clock models.
//
// Summary
// -------
// A clock provides a separate "clock time" axis and scheduling primitives that
// mirror OMNeT++’s simulation-time scheduling, but are driven by the clock’s
// own rate, offset, and adjustments. Client modules interact with the clock via
// the IClock C++ interface; events are transparently (re)scheduled on behalf of
// the client and delivered when the clock’s condition is met.
//
// Key semantics (see IClock for exact definitions)
// ------------------------------------------------
// - The clock defines a mapping C(t) from simulation time to clock time that is
//   non-decreasing in t. Explicit adjustments (step/offset/origin change) may
//   introduce jumps; rate/drift changes affect the slope.
// - Conversions handle discontinuities with a lower/upper bound convention
//   (C++ methods take a `lowerBound` flag to disambiguate).
// - Absolute scheduling (“at clock time c”): arrival simulation time shifts if
//   the clock is stepped or its origin is changed later; rate/drift changes also
//   affect it.
// - Relative scheduling (“after clock duration Δc”): arrival simulation time is
//   insensitive to later absolute steps but still affected by rate/drift changes.
//
// Typical use
// -----------
// - Instantiate a clock module (implementing this interface) as a submodule.
// - Pass its module path to users (e.g., NICs, protocol modules) via a parameter.
// - In C++, derive users from ClockUserModuleBase / ClockUserModuleMixin to
//   call scheduleClockEventAt()/scheduleClockEventAfter(), and the conversion
//   helpers computeClockTimeFromSimTime()/computeSimTimeFromClockTime().
//
// Notes
// -----
// - Using a per-interface or per-node clock allows modeling clock skew/drift and
//   observing their impact on protocol timing (e.g., link-layer backoffs, TSN gates).
// - See also ~IOscillator for the underlying tick-counting abstraction used by some
//   clock models.
//
// @see ~IOscillator
//
moduleinterface IClock
{
    parameters:
        @display("i=block/timer");
}

