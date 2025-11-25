//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.model;

import inet.clock.base.ClockBase;
import inet.clock.contract.IClock;

//
// Ideal clock: clock time equals simulation time.
//
// Summary
// -------
// Provides a trivial mapping C(t) = t (no offset, no drift, no steps).
// Useful as a baseline or when protocol logic should use simulation time
// directly via the ~IClock API.
//
// Semantics
// ---------
// - getClockTime() == simTime() always.
// - computeClockTimeFromSimTime(t) = t.
// - computeSimTimeFromClockTime(c) = c.
// - Scheduling “at” a clock time is equivalent to scheduling at the same
//   simulation time; relative “after” delays behave identically.
//
// Notes
// -----
// - No quantization/rounding beyond the simulator’s time precision.
// - No oscillator is involved.
//
module IdealClock extends ClockBase like IClock
{
    parameters:
        @class(IdealClock);
}

