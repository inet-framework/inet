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

import inet.physicallayer.common.Radio;

module Ieee80211Radio extends Radio
{
    parameters:
        int channelNumber = default(0);
        string opMode @enum("b","g","a","p") = default("g");
        string preambleMode @enum("long", "short") = default("long");
        double carrierFrequency @unit(Hz) = default(2.412GHz);
        double bandwidth @unit(Hz) = default(2MHz);
        antennaType = default("IsotropicAntenna");
        transmitter.bitrate = default(11Mbps);
        transmitter.headerBitLength = default(192b);
        transmitter.power = default(20mW);
        receiver.sensitivity = default(-85dBm);
        receiver.energyDetection = default(-85dBm);
        receiver.snirThreshold = default(4dB);
        *.carrierFrequency = carrierFrequency;
        *.bandwidth = bandwidth;
        *.channelNumber = channelNumber;
        *.opMode = opMode;
        *.preambleMode = preambleMode;
        @class(Ieee80211Radio);
        @signal[radioChannelChanged](type=long);
        @statistic[radioChannel](title="Radio channel"; source=radioChannelChanged; record=histogram,vector; interpolationmode=sample-hold);
}
