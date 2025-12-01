//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.model;

import inet.clock.contract.IClock;

//
// Settable (step-able) clock with adjustable oscillator compensation.
//
// Summary
// -------
// Extends ~OscillatorBasedClock with the ability to step the clock time to an
// explicitly provided value (forward or backward) and to apply a new near-1
// oscillator-compensation factor. Useful for scripted corrections via
// ~ScenarioManager or programmatic changes from C++.
//
// Semantics
// ---------
// - Stepping the clock introduces a discontinuity in the clock-time mapping.
//   Overdue events (whose target time becomes < now after the step) are handled
//   according to `defaultOverdueClockEventHandlingMode`:
//     * "execute" : run overdue events immediately (in order);
//     * "skip"    : drop/cancel overdue events;
//     * "error"   : report an error.
// - The oscillator compensation is expressed in ppm; positive values make the
//   clock run faster (effective factor x ≈ 1 + ppm·1e−6).
//
// Scripting
// ---------
// - Can be controlled from C++ or with ScenarioManager commands, e.g.:
//     <set-clock module="path.to.clock" time="1.23s"/>
//
// Underlying clock
// ----------------
// - Internally wraps an `underlyingClock` (defaults to ~OscillatorBasedClock)
//   so setups can share code while exposing step/compensation at this layer.
//
// Signals / Statistics
// --------------------
// - Emits `oscillatorCompensationChanged` (ppm) when compensation changes.
// - Records a vector statistic of the same name.
//
// @see ~ScenarioManager, ~inet.clock.contract.IClock, ~inet.clock.model.OscillatorBasedClock
//
module SettableClock extends OscillatorBasedClock
{
    parameters:
        double initialOscillatorCompensation @unit(ppm) = default(0ppm); // Initial compensation (x ≈ 1 + ppm·1e−6; + speeds up)
        string defaultOverdueClockEventHandlingMode @enum("execute","skip","error") = default("execute"); // Policy for overdue events after a step
        @class(SettableClock);
        @signal[oscillatorCompensationChanged](type=double);
        @statistic[oscillatorCompensationChanged](title="oscillator compensation changed"; record=vector; interpolationmode=sample-hold; unit=ppm);
    submodules:
        underlyingClock: <default("OscillatorBasedClock")> like IClock if typename != "" {
            parameters:
                oscillator.typename = default("");
                oscillatorModule = default("^.oscillator");
                useFutureEventSet = default(parent.useFutureEventSet);
                @display("p=100,200");
        }
}

