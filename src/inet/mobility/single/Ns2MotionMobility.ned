//
// Copyright (C) 2008 Alfonso Ariza
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
simple Ns2MotionMobility extends MovingMobilityBase
{
    parameters:
        string traceFile; // the BonnMotion trace file
        int nodeId; // selects line in trace file; -1 gets substituted to parent module's index
        double scrollX @unit(m) = default(0m);
        double scrollY @unit(m) = default(0m);
        @class(Ns2MotionMobility);
}

