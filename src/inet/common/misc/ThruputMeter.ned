//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common.misc;

import inet.common.SimpleModule;
import inet.common.IHook;

//
// Thruput measurement utility module.
//
simple ThruputMeter extends SimpleModule like IHook
{
    parameters:
        @class(ThruputMeter);
        @networkNode();
        double startTime @unit(s) = default(0s);
        int batchSize = default(50);
        double maxInterval @unit(s) = default(1s);
        @display("i=block/timer");
    gates:
        input in;
        output out;
}

