//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

//
// Implements the free space path loss model, which models signal attenuation
// over distance according to the Friis transmission equation. Calculates signal
// power loss as a function of distance, wavelength, and configurable parameters.
// The alpha parameter controls the path loss exponent (default 2), while the
// systemLoss parameter accounts for hardware inefficiencies. Provides an ideal
// baseline model for wireless propagation in environments with line-of-sight
// and no obstacles.
//
module FreeSpacePathLoss extends Module like IPathLoss
{
    parameters:
        double alpha = default(2);
        double systemLoss @unit(dB) = default(0dB);
        @class(FreeSpacePathLoss);
        @display("i=block/control");
}

