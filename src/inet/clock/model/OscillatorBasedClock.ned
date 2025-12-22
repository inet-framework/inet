//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.model;

import inet.clock.base.ClockBase;
import inet.clock.contract.IClock;
import inet.clock.contract.IOscillator;

//
// Oscillator-driven clock.
//
// Summary
// -------
// A clock that derives its time from an underlying ~IOscillator. The oscillator may drift, i.e., its actual tick
// spacing can vary over simulation time. The clock maps simulation time to clock time by counting oscillator ticks
// and using the *nominal* tick length as the tick-to-time conversion. No per-tick events are generated.
//
// Semantics
// ---------
// - Let the oscillator expose:
//     N(Δt): ticks in (0, Δt] from its computation origin,
//     I(n) : minimal Δt ≥ 0 with N(Δt) ≥ n.
// - The clock maintains a (cos, coc) origin pair (simulation/clock time). From there, simulation time to clock time
//   and clock time to simulation time conversions are computed using N/I and the nominal tick length.
// - Absolute (“at clock time”) vs. relative (“after clock duration”) scheduling behave as specified by ~IClock.
//   Oscillator rate/origin changes affect the arrival simulation time accordingly; absolute steps shift “at” events,
//   relative delays remain anchored but follow the new rate going forward.
//
// Granularity & rounding
// ----------------------
// - The clock quantizes requested targets to a grid G before scheduling, where
//     G := (clockGranularity != 0s) ? clockGranularity : nominalTickLength(osc).
// - roundingMode applies to both absolute targets and relative delays:
//     * down   : floor to grid G
//     * up     : ceil  to grid G
//     * closer : nearest grid point (tie-breaking as implemented; typically half-up)
//     * none   : no rounding (requested value used as is)
// - Use a smaller clockGranularity than the nominal tick length if you need to represent fine compensation steps more accurately.
//
// Event management
// ----------------
// - useFutureEventSet = true:
//     Events are scheduled directly into the OMNeT++ FES.
// - useFutureEventSet = false:
//     The clock maintains its own pending list and executes events when the oscillator advances; this requires the
//     oscillator to emit its tick/state signals.
//
// Display string format directives
// --------------------------------
//  -s : origin simulation time (cos)
//  -c : origin clock time (coc)
//
module OscillatorBasedClock extends ClockBase like IClock
{
    parameters:
        displayStringTextFormat = default("%t (%c)"); // Determines the text that is written on top of the submodule
        string oscillatorModule = default(".oscillator");
        double initialClockTime @unit(s) = default(0s); // Clock time at the simulation time when the module is initialized
        string roundingMode @enum("down","up","closer","none") = default("up"); // When scheduling to a clock time or with a clock delay this clock uses rounding with respect to the nominal tick length
        bool useFutureEventSet = default(true);        // true: schedule in FES; false: internal list (requires oscillator signals)
        double clockGranularity @unit(s) = default(0s);// Scheduling grid G; 0s ⇒ use oscillator nominal tick length
        @class(OscillatorBasedClock);
    submodules:
        oscillator: <default("ConstantDriftOscillator")> like IOscillator if typename != "" {
            @display("p=100,100");
            emitNumTicksSignal = default(!parent.useFutureEventSet); // needed when not using FES
        }
}
