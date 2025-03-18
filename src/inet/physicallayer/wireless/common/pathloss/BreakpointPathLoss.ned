//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

//
// Path loss model that uses a dual-slope approach with different path loss
// exponents for different distance ranges. Models signal attenuation with a
// breakpoint distance that divides propagation into two regions, each with its
// own path loss characteristics. Particularly useful for environments where
// propagation behavior changes significantly at a certain distance, such as
// indoor/outdoor transitions or urban scenarios with varying obstacle densities.
//
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
