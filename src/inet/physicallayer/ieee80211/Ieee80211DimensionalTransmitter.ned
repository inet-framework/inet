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

package inet.physicallayer.ieee80211;

import inet.physicallayer.apsk.APSKDimensionalTransmitter;

//
// This transmitter model produces IEEE 802.11 transmissions that have
// dimensional transmission power (that changes over time and/or frequency) in
// their analog representation. The bit domain, symbol domain, and sample
// domains of the transmissions are not represented.
//
// See also ~Ieee80211DimensionalReceiver, ~Ieee80211DimensionalRadio, and
// ~Ieee80211DimensionalRadioMedium.
//
// TODO: remove APSK dependency and extend Ieee80211TransmitterBase, factor out common part
module Ieee80211DimensionalTransmitter extends APSKDimensionalTransmitter
{
    parameters:
        // TODO: remove parameters when base module is changed
        int channelNumber;
        string opMode @enum("b","g","a","p");
        string preambleMode @enum("long", "short");
        @class(Ieee80211DimensionalTransmitter);
}
