//
// Copyright (C) Andras Varga
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
// Generic radio module. The implementation is largely based on the
// Mobility Framework's ~SnrEval and Decider modules.
//
// @see ~IRadio
// @author Andras Varga
//
simple GenericRadio extends Radio
{
    parameters:
        @display("i=block/wrxtx");
        
        radioModel = "GenericRadioModel";  // specify the radio model responsible for modulation, error correction and frame length calculation
        double snirThreshold @unit("dB") = default(4dB); // if signal-noise ratio is below this threshold, frame is considered noise (in dB)
        int headerLengthBits @unit(b); // length of physical layer framing (preamble, etc)
        double bandwidth @unit("Hz"); // signal bandwidth, used for bit error calculation
        string modulation; // "BPSK", "16-QAM", "256-QAM" or "null"; selects bit error calculation method
}
