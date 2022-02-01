//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.physicallayer.wireless.ieee802154.packetlevel;

import inet.physicallayer.wireless.common.medium.RadioMedium;

module Ieee802154NarrowbandScalarRadioMedium extends RadioMedium
{
    parameters:
        backgroundNoise.typename = default("IsotropicScalarBackgroundNoise");

        mediumLimitCache.centerFrequency = 2450 MHz;

        // 802.15.4-2006, page 266
        pathLoss.typename = default("BreakpointPathLoss");
        pathLoss.breakpointDistance = 8 m;
        pathLoss.l01 = 40.2;
        pathLoss.alpha1 = 2;
        pathLoss.l02 = 58.5;
        pathLoss.alpha2 = 3.3;

        // Reflects the thermal noise for the receiver sensitivity
        // TODO couple with sensitivity
        backgroundNoise.power = default(-96.616dBm);
        backgroundNoise.dimensions = default("time");
}

