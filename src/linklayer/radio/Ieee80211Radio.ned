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


package inet.linklayer.radio;

//
// Physical layer for the IEEE 802.11 models. Its external interface
// (including gates and how it communicates with other modules)
// is the same as the Radio module's.
//
// The implementation is largely based on the Mobility Framework's
// SnrEval80211 and Decider80211 modules.
//
// @see Radio
// @author Andras Varga
//
simple Ieee80211Radio like Radio
{
    parameters:
        int channelNumber = default(0); // channel identifier
        double transmitterPower @unit("mW") = default(20mW); // power used for transmission of messages (in mW)
        double bitrate @unit("bps"); // (in bits/s)
        double thermalNoise @unit("dBm") = default(-110dBm); // base noise level (dBm)
        double pathLossAlpha = default(2); // used by the path loss calculation
        double shadowingDeviation @unit("dB") = default(0dB); // used by the shadowing model calculation
        double snirThreshold @unit("dB") = default(4dB); // if signal-noise ratio is below this threshold, frame is considered noise (in dB)
        double sensitivity @unit("mW"); // received signals with power below sensitivity are ignored
        @display("i=block/wrxtx");
    gates:
        input uppergateIn @labels(PhyControlInfo/down,Ieee80211Frame);   // from higher layer protocol (MAC)
        output uppergateOut @labels(PhyControlInfo/up,Ieee80211Frame); // to decider (decider connects to higher layer protocol, i.e. the MAC)
        input radioIn @labels(AirFrame); // to receive frames (AirFrame) on the radio channel
}
