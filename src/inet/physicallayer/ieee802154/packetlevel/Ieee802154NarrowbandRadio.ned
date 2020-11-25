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

import inet.physicallayer.base.packetlevel.FlatRadioBase;

module Ieee802154NarrowbandRadio extends FlatRadioBase
{
    parameters:
        string signalAnalogRepresentation @enum("scalar", "dimensional") = default("scalar");
        transmitter.typename = default(signalAnalogRepresentation == "scalar" ? "Ieee802154NarrowbandScalarTransmitter" : signalAnalogRepresentation == "dimensional" ? "Ieee802154NarrowbandDimensionalTransmitter" : "");
        receiver.typename = default(signalAnalogRepresentation == "scalar" ? "Ieee802154NarrowbandScalarReceiver" : signalAnalogRepresentation == "dimensional" ? "Ieee802154NarrowbandDimensionalReceiver" : "");

        centerFrequency = default(2450 MHz);

        // B_20dB ATmega256RFR2 (page 564)
        bandwidth = default(2.8 MHz);

        // 802.15.4-2006 (page 28)
        *.bitrate = default(250 kbps);

        // PHY Header (without preamble), 802.15.4-2006 (page 43)
        // 1 octet SFD
        // 7 bit Frame length
        // 1 bit Reserved
        *.headerLength = (1*8 + 7 + 1) * 1 b;

        // Preamble
        // 4 octets Preamble
        // 1 symbol of 16us -> 4 bit
        transmitter.preambleDuration = (4*8/4) * 16 us;

        // RSSI sensitivity (ATmega256RFR2, page 566)
        receiver.energyDetection = default(-90dBm);

        // Receiver sensitivity (ATmega256RFR2, page 565)
        // TODO That is not quite true, because sensitivity
        //      is defined as the input signal power that yields
        //      a PER < 1% for a PSDU of 20 octets, but INET handles it
        //      as minimum reception power.
        receiver.sensitivity = default(-100dBm);

        // There is no fixed boundary, because of the
        // DSSS and the capture effect. Taking the sensitivity minus some
        // arbitrary value as an approximate guess.
        receiver.minInterferencePower = default(-120dBm);

        // Minimum SNIR
        // -8 dB results into 98% PER for a PSDU of 20 octets
        receiver.snirThreshold = default(-8 dB);

        // TX Output power (typ. 3.5 dBm, ATmega256RFR2, page 564)
        transmitter.power = default(2.24mW);

        @class(FlatRadioBase);
}

