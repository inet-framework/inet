//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

module BreakpointPathLoss like IPathLoss
{
    parameters:
        double l01;
        double l02;
        double alpha1;
        double alpha2;
        double breakpointDistance @unit(m);
        @class(BreakpointPathLoss);
}

