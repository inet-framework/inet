//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.model;

import inet.clock.contract.IClock;

//
// Models a clock which can be set to a different clock time. The clock time
// can be set from C++ or using a <set-clock module="..." time="..."/> command
// in a `ScenarioManager` script.
//
// @see ~ScenarioManager
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

