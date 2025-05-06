//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

//
// Path loss model that uses a dual-slope approach with different path loss
// exponents for different distance ranges. Models signal attenuation with a
// breakpoint distance that divides propagation into two regions, each with its
// own path loss characteristics. Particularly useful for environments where
// propagation behavior changes significantly at a certain distance, such as
// indoor/outdoor transitions or urban scenarios with varying obstacle densities.
//
module BreakpointPathLoss extends Module like IPathLoss
{
    parameters:
        double l01 @unit(dB);  // Initial path loss for the near-distance region
        double l02 @unit(dB);  // Initial path loss for the far-distance region
        double alpha1;  // Path loss exponent for the near-distance region
        double alpha2;  // Path loss exponent for the far-distance region
        double breakpointDistance @unit(m);
        @class(BreakpointPathLoss);
}

