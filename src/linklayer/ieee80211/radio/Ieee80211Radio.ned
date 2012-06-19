//
// Copyright (C) Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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


package inet.linklayer.ieee80211.radio;
import inet.linklayer.radio.Radio;
//
// Physical layer for the IEEE 802.11 models. Its external interface
// (including gates and how it communicates with other modules)
// is the same as the Radio module's.
//
// @see Radio
// @author Alfonso Ariza Quintana
//
simple Ieee80211Radio extends Radio
{
    parameters:
        @display("i=block/wrxtx");
        radioModel = "Ieee80211RadioModel";  // specify the radio model responsible for modulation, error correction and frame length calculation
        double snirThreshold @unit("dB") = default(4dB); // if signal-noise ratio is below this threshold, frame is considered noise (in dB)
        string berTableFile = default("");
        string phyOpMode @enum("b","g","a","p") = default("g");
        string wifiPreambleMode @enum("LONG","SHORT") = default("LONG"); // Wifi preambre mode Ieee 2007, 19.3.2
        string errorModel @enum("YansModel","NistModel") = default("NistModel");
        int btSize @unit("b") = default(8192b);// test size frame for Airtime Link Metric
        bool airtimeLinkComputation = default(false);

        bool AutoHeaderSize = default(false); // in the receiver the radio model compute the header size in function of timers and bitrate
}

