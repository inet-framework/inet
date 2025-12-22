//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.model;

import inet.clock.contract.IClock;

//
// This module represents a settable (step-able) clock with adjustable oscillator compensation.
//
// Extends ~OscillatorBasedClock with the ability to step the clock time to an explicitly provided value (forward or
// backward) and to apply a new oscillator-compensation factor. Stepping the clock introduces a discontinuity in the
// clock-time mapping. Overdue events (whose target time becomes < now after the step) can be handled in three ways:
//  - execute : run overdue events immediately (in order)
//  - skip    : drop/cancel overdue events
//  - error   : terminate simulation and report an error
//
// The clock time can be set from C++ or by a clock servo module or by a time synchronization protocol or using a
// <set-clock module="..." time="..."/> command in a `ScenarioManager` script.
//
// @see ~IClockServo, ~ScenarioManager
//
module SettableClock extends OscillatorBasedClock
{
    parameters:
        double initialOscillatorCompensation @unit(ppm) = default(0ppm);
        string defaultOverdueClockEventHandlingMode @enum("execute","skip","error") = default("execute");
        @class(SettableClock);
    submodules:
        underlyingClock: <default("OscillatorBasedClock")> like IClock if typename != "" {
            parameters:
                oscillator.typename = "";
                oscillatorModule = default("^.oscillator");
                useFutureEventSet = default(parent.useFutureEventSet);
                @display("p=100,200");
        }
}

