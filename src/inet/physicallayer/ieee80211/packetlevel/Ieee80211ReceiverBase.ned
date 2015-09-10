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

import inet.physicallayer.base.packetlevel.NarrowbandReceiverBase;

//
// This receiver model serves as the base module for IEEE 802.11 receivers. It
// supports switching channels, configuring different operation modes, and
// preamble modes.
//
// See also ~Ieee80211TransmitterBase, ~Ieee80211ScalarReceiver, and
// ~Ieee80211DimensionalReceiver.
//
module Ieee80211ReceiverBase extends NarrowbandReceiverBase
{
    parameters:
        string opMode @enum("a", "b", "g", "n", "p");
        string bandName @enum("2.4 GHz", "5 GHz");
        int channelNumber;
        modulation = default("BPSK"); // TODO: this is simply wrong
        errorModelType = default("Ieee80211NistErrorModel");
        // TODO: remove when opMode is removed from the error model
        *.opMode = opMode;
}
