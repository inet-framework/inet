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

//
// This radio medium model uses scalar transmission power in the analog
// representation. It must be used in conjunction with the
// ~Ieee80211ScalarRadio model.
//
// @see ~Ieee80211ScalarRadio, ~Ieee80211ScalarTransmitter,
// ~Ieee80211ScalarReceiver, ~ScalarAnalogModel.
//
module Ieee80211ScalarRadioMedium extends Ieee80211RadioMedium
{
    parameters:
        analogModel.typename = default("ScalarAnalogModel");
        backgroundNoise.typename = default("IsotropicScalarBackgroundNoise");
}

