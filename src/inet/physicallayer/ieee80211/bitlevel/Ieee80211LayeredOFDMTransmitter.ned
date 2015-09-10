//
// Copyright (C) 2014 OpenSim Ltd
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

package inet.physicallayer.ieee80211.bitlevel;

import inet.physicallayer.contract.packetlevel.ITransmitter;
import inet.physicallayer.contract.bitlevel.IDigitalAnalogConverter;
import inet.physicallayer.contract.bitlevel.IEncoder;
import inet.physicallayer.contract.bitlevel.IModulator;
import inet.physicallayer.contract.bitlevel.IPulseShaper;

//
//
// This module implements an IEEE 802.11 OFDM transmitter.
// The implemenation details are based on the following standard:
// IEEE Std 802.11-2012 PART 11: WIRELESS LAN MAC AND PHY SPECIFICATIONS

// If the compliant mode is set to true then the transmitter
// works in compliant IEEE 802.11 modes (can be found in the
// standard). The current mode depends on the IEEE 802.11 MAC,
// thus, in this case it is only allowed to set the power.
// If it is configured to be a non-compliant transmitter then
// you can freely set your own submodules: modulator, encoder,
// etc., or other parameters such as carrier frequency, banwidth,
// etc.

// The level of detail parameter determines which submodules of
// the transmitter will be used:
//
// Level (domain)|                        Submodules
// ======================================================================
//    PACKET     |-
//               --------------------------------------------------------
//    BIT        | signal encoder, data encoder
//               --------------------------------------------------------
//    SYMBOL     | signal modulator, data modulator + bit level modules
//               --------------------------------------------------------
//    SAMPLE     | pulse shaper + bit, symbol level modules + (digital/
//               | analog converter)
//               --------------------------------------------------------
//
// Important: We have no default module implementation for sample level
// features.
//
// Note that if a digital/analog converter is provided for the
// transmitter then it will be only used at sample level, otherwise,
// since analog representation is obligatory, a simplified, built-in
// (scalar) analog model is used at all levels.
//
// Note that in non-compliant mode, whatever the level of detail is,
// you must always set all submodules. If the current level of detail
// does not demand a specific submodule it is necessary and sufficient
// to implement those methods of that submodule which provide metadata
// about its settings.
//
// For example, if the level of detail is bit level, you don't have to
// implement a complete modulator, it is enough to implement a dummy
// modulator that can give information about its subcarrier modulation
// (e.g. BPSK).
//
//
module Ieee80211LayeredOFDMTransmitter like ITransmitter
{
    parameters:
        bool isCompliant = default(true);
        string levelOfDetail @enum("packet","bit","symbol","sample") = default("symbol");
        string signalEncoderType = default("");
        string dataEncoderType = default("");
        string signalModulatorType = default("");
        string dataModulatorType = default("");
        string pulseShaperType = default("");
        string digitalAnalogConverterType = default("");
        double channelSpacing @unit(Hz) = default(0Hz/0);
        double power @unit(W);
        double carrierFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        @class(Ieee80211LayeredOFDMTransmitter);

        @display("i=block/tx");
    submodules:
        signalEncoder: <signalEncoderType> like IEncoder if signalEncoderType != "" {
            @display("p=100,50");
        }
        dataEncoder: <dataEncoderType> like IEncoder if dataEncoderType != "" {
            @display("p=226,50");
        }
        dataModulator: <dataModulatorType> like IModulator if dataModulatorType != "" {
            @display("p=100,150");
        }
        signalModulator: <signalModulatorType> like IModulator if signalModulatorType != "" {
            @display("p=226,150");
        }
        pulseShaper: <pulseShaperType> like IPulseShaper if pulseShaperType != "" {
            @display("p=100,250");
        }
        digitalAnalogConverter: <digitalAnalogConverterType> like IDigitalAnalogConverter if digitalAnalogConverterType != "" {
            @display("p=100,350");
        }
}
