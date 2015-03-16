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
// This radio medium model uses dimensional transmission power (that changes
// over time and/or frequency) in the analog representation. It must be used in
// conjunction with the ~Ieee80211DimensionalRadio model.
//
// See also ~Ieee80211DimensionalRadio, ~Ieee80211DimensionalTransmitter,
// ~Ieee80211DimensionalReceiver, and ~DimensionalAnalogModel.
//
module Ieee80211DimensionalRadioMedium extends Ieee80211RadioMedium
{
    parameters:
        analogModelType = default("DimensionalAnalogModel");
        backgroundNoiseType = default("IsotropicDimensionalBackgroundNoise");
}
