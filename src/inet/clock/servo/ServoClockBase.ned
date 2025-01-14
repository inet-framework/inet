//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.servo;

import inet.clock.model.OscillatorBasedClock;

// TODO: Write documentation
module ServoClockBase extends OscillatorBasedClock
{
    parameters:
        string defaultOverdueClockEventHandlingMode @enum("execute","skip","error") = default("error");
        @class(ServoClockBase);
}

