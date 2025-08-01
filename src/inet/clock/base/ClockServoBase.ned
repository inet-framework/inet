//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.clock.base;

import inet.common.Module;

module ClockServoBase extends Module
{
    parameters:
        string clockModule;
        @class(ClockServoBase);
        @display("i=block/control");
}

