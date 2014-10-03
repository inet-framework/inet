//
// Copyright (C) 2014 OpenSim Ltd.
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

package inet.physicallayer.ieee80211.layered;

import inet.physicallayer.contract.IFECCoder;
import inet.physicallayer.contract.IInterleaver;
import inet.physicallayer.layered.LayeredEncoder;


// Note: this is not a general Ieee80211Encoder; it works in 36Mb/s mode.
module Ieee80211LayeredEncoder extends LayeredEncoder
{
    parameters:
        fecType = default("ConvolutionalCoder");
        string signalFECType = default("ConvolutionalCoder");
        serializerType = default("Ieee80211Serializer");
        scramblerType = default("Ieee80211Scrambler");
        interleaverType = default("Ieee80211Interleaver");
        string signalInterleaverType = default("Ieee80211Interleaver");
        @display("bgb=270,313");
        @class(Ieee80211LayeredEncoder);

    submodules:
        signalFECEncoder: <signalFECType> like IFECCoder if signalFECType != "" {
            @display("p=186,168");
        }
        signalInterleaver: <signalInterleaverType> like IInterleaver if signalInterleaverType != "" {
            @display("p=186,244");
        }
}
