//
// Copyright (C) 2013 OpenSim Ltd
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

package inet.physicallayer.ieee80211.packetlevel;

import inet.physicallayer.common.packetlevel.RadioMedium;

//
// This radio medium model is part of thee IEEE 802.11 physical layer model. It
// must be used in conjunction with the ~Ieee80211Radio model or other derived
// models.
//
// This model provides sensible default values for the radio medium parameters
// to be used in IEEE 802.11 simulations.
//
// See also ~Ieee80211ScalarRadioMedium, and ~Ieee80211DimensionalRadioMedium.
//
module Ieee80211RadioMedium extends RadioMedium
{
    parameters:
        propagationType = default("ConstantSpeedPropagation");
        pathLossType = default("FreeSpacePathLoss");
        backgroundNoise.power = default(-110dBm);
        mediumLimitCache.carrierFrequency = default(2.4GHz);
        mediumLimitCache.minReceptionPower = default(-85dBm);
        mediumLimitCache.minInterferencePower = default(-110dBm);
}
