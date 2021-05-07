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

import inet.physicallayer.base.packetlevel.FlatRadioBase;

//
// This radio model is part of the IEEE 802.11 physical layer model. It supports
// multiple channels, different operation modes, and preamble modes. It must be
// used in conjunction with the ~Ieee80211RadioMedium model or other derived
// models.
//
// This model provides sensible default values for the radio parameters to be
// used in IEEE 802.11 simulations. It also supports switching channels via
// ~ConfigureRadioCommand, or with direct C++ function calls.
//
// @see ~Ieee80211ScalarRadio, ~Ieee80211DimensionalRadio.
//
//# TODO: check this Table 18-14—Receiver performance requirements
module Ieee80211Radio extends FlatRadioBase
{
    parameters:
        string signalAnalogRepresentation @enum("scalar", "dimensional") = default("scalar");
        string opMode @enum("a", "b", "g(erp)", "g(mixed)", "n(mixed-2.4Ghz)", "p", "ac") = default("g(mixed)"); // operation mode
        string bandName @enum("2.4 GHz", "5 GHz", "5 GHz (20 MHz)", "5 GHz (40 MHz)", "5 GHz (80 MHz)", "5 GHz (160 MHz)", "5.9 GHz") = default("2.4 GHz"); // band name
        int channelNumber = default(0);                                 // initial channel number within the band (TODO: this is offset by 1)
        string crcMode @enum("declared","computed") = default("declared");
        transmitter.typename = default(signalAnalogRepresentation == "scalar" ? "Ieee80211ScalarTransmitter" : signalAnalogRepresentation == "dimensional" ? "Ieee80211DimensionalTransmitter" : "");
        receiver.typename = default(signalAnalogRepresentation == "scalar" ? "Ieee80211ScalarReceiver" : signalAnalogRepresentation == "dimensional" ? "Ieee80211DimensionalReceiver" : "");
        centerFrequency @unit(Hz) = default(2.412GHz);                 // the center frequency of the band is automatically set by the radio
        bandwidth @unit(Hz) = default(nan MHz);
        transmitter.preambleDuration = default(nan s);
        transmitter.bitrate = default(11Mbps);
        transmitter.headerLength = default(192b);
        transmitter.power = default(20mW);
        receiver.bandwidth = default(22 MHz); // the highest value
        receiver.sensitivity = default(-85dBm);
        receiver.energyDetection = default(-85dBm);
        receiver.snirThreshold = default(4dB);
        *.opMode = this.opMode;
        *.bandName = this.bandName;
        *.channelNumber = this.channelNumber;
        @class(Ieee80211Radio);
        @signal[radioChannelChanged](type=long);
        @statistic[radioChannel](title="Radio channel"; source=radioChannelChanged; record=histogram,vector; interpolationmode=sample-hold);
}

