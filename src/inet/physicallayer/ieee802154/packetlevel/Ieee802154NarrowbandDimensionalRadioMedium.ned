//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.physicallayer.ieee802154.packetlevel;

import inet.physicallayer.common.packetlevel.RadioMedium;

module Ieee802154NarrowbandDimensionalRadioMedium extends RadioMedium
{
    parameters:
        analogModel.typename = default("DimensionalAnalogModel");
        backgroundNoise.typename = default("IsotropicDimensionalBackgroundNoise");

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

