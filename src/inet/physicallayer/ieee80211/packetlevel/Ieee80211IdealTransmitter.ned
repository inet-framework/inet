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
// See also ~IdealReceiver, ~Ieee80211IdealRadio, and
// ~IdealRadioMedium.
//
module Ieee80211IdealTransmitter extends Ieee80211TransmitterBase
{
    parameters:
        double maxCommunicationRange @unit(m);                                 // the maximum range where the reception of transmissions produced by this transmitter is possible
        double maxInterferenceRange @unit(m) = default(maxCommunicationRange); // the maximum range where transmissions produced by this transmitter interferes with other transmissions
        double maxDetectionRange @unit(m) = default(maxInterferenceRange);     // the maximum range where transmissions produced by this transmitter can be detected at all
        @class(Ieee80211IdealTransmitter);
}
