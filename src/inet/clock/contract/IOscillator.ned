//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.contract;

//
// Interface for oscillator models.
//
// Summary
// -------
// An oscillator provides a conceptual sequence of tick instants used by clocks
// to relate simulation time to tick counts. For performance, oscillators do not
// emit one event per tick; instead they expose two mapping operations via the
// C++ ~IOscillator API:
//
//   N(Δt): number of ticks in the interval (0, Δt] measured from the
//          computation origin.
//   I(n) : minimal Δt ≥ 0 such that N(Δt) ≥ n (with I(0) = 0).
//
// Semantics
// ---------
// - The computation origin o (o ≤ now) is the reference simulation time from
//   which Δt is measured. The origin need not coincide with a tick.
// - A tick exactly at Δt is counted by N(Δt); a tick at the origin is not.
// - N is non-decreasing and right-continuous with unit jumps at tick instants.
// - I is the generalized right-inverse of N (monotone, I(0)=0).
// - The nominal tick length reported by the implementation is a reference
//   parameter; actual tick spacing may vary (e.g., due to drift/phase changes).
//
// Typical use
// -----------
// - A clock model (see ~IClock) uses an oscillator to convert between a time
//   interval and a tick count without generating per-tick events.
// - Protocol or hardware models that need tick-accurate timing can query
//   N(Δt) / I(n) through the ~IOscillator C++ interface.
//
// Notes
// -----
// - This is a contract; concrete oscillator modules define how drift, phase,
//   and parameter changes affect N/I, and when they update their origin/state.
//
// @see ~IClock
//
moduleinterface IOscillator
{
    parameters:
        @display("i=block/tunnel");
}

