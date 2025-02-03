//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.servo;

import inet.clock.model.OscillatorBasedClock;

// A base module for servo clocks that extends the OscillatorBasedClock.
// This module serves as a foundation for implementing servo clock mechanisms,
// providing a structured approach for handling clock adjustments and overdue events.
module ServoClockBase extends OscillatorBasedClock
{
    parameters:
        string defaultOverdueClockEventHandlingMode @enum("execute","skip","error") = default("error");
        @class(ServoClockBase);
}

