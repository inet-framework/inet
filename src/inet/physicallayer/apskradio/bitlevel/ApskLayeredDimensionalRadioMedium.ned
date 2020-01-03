//
// Copyright (C) 2014 OpenSim Ltd.
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

package inet.physicallayer.apskradio.bitlevel;

import inet.physicallayer.common.packetlevel.RadioMedium;

//
// This medium model is used by a simple hypothetical layered radio.
//
// @see ~ApskLayeredDimensionalRadio, ~ApskLayeredTransmitter, ~ApskLayeredReceiver.
//
module ApskLayeredDimensionalRadioMedium extends RadioMedium
{
    parameters:
        analogModel.typename = default("LayeredDimensionalAnalogModel");
        backgroundNoise.typename = default("IsotropicDimensionalBackgroundNoise");
        backgroundNoise.power = default(-110dBm);
        mediumLimitCache.centerFrequency = default(2.4GHz);
        mediumLimitCache.minReceptionPower = default(-85dBm);
        mediumLimitCache.minInterferencePower = default(-110dBm);
}

