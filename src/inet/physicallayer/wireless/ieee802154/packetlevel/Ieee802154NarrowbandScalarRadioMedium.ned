//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//TODO review
package inet.physicallayer.wireless.ieee802154.packetlevel;

import inet.physicallayer.wireless.common.medium.RadioMedium;

//
// Radio medium model for IEEE 802.15.4 narrowband networks that uses scalar
// representation for signal power. Configured with parameters from the IEEE
// 802.15.4-2006 standard, including a dual-slope breakpoint path loss model with
// a breakpoint distance of 8 meters. Provides a computationally efficient
// simulation of signal propagation in the 2.4 GHz band with appropriate
// background noise and path loss characteristics for low-power wireless personal
// area networks.
//
module Ieee802154NarrowbandScalarRadioMedium extends RadioMedium
{
    parameters:
        backgroundNoise.typename = default("IsotropicScalarBackgroundNoise");

        mediumLimitCache.centerFrequency = 2450 MHz;

        // 802.15.4-2006, page 266
        pathLoss.typename = default("BreakpointPathLoss");
        pathLoss.breakpointDistance = 8 m;
        pathLoss.l01 = 40.2 dB;
        pathLoss.alpha1 = 2;
        pathLoss.l02 = 58.5 dB;
        pathLoss.alpha2 = 3.3;

        // Reflects the thermal noise for the receiver sensitivity
        // TODO couple with sensitivity
        backgroundNoise.power = default(-96.616dBm);
}

