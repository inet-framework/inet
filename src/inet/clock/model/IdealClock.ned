//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.model;

import inet.clock.base.ClockBase;
import inet.clock.contract.IClock;

//
// Models a clock where the clock time is identical to the simulation time.
//
module IdealClock extends ClockBase like IClock
{
    parameters:
        @class(IdealClock);
}

