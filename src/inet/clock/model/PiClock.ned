//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.model;

//
// Models a clock which can be set to a different clock time. The clock time
// can be set from C++ or using a <set-clock module="..." time="..."/> command
// in a ScenarioManager script.
//
// @see ~ScenarioManager
//
module PiClock extends OscillatorBasedClock
{
    parameters:
        string defaultOverdueClockEventHandlingMode @enum("execute","skip","error") = default("error");
        @signal[kp](type=double);
        @statistic[kp](title="kp value"; record=vector; interpolationmode=linear);
        @signal[ki](type=double);
        @statistic[ki](title="ki value"; record=vector; interpolationmode=linear);
        @class(PiClock);
}

