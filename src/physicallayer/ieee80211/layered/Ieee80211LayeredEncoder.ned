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

import inet.physicallayer.layered.LayeredEncoder;
import inet.physicallayer.contract.IForwardErrorCorrection;

module Ieee80211LayeredEncoder extends LayeredEncoder
{
    parameters:
        string signalForwardErrorCorrectionType = "ConvolutionalCoder"; // rename
        serializerType = "Ieee80211Serializer";
        scramblerType = "Ieee80211Scrambler";
        forwardErrorCorrectionType = "ConvolutionalCoder"; // rename
        interleaverType = "Ieee80211Interleaver";
        
    submodules:
        signalForwardErrorCorrection: <signalForwardErrorCorrectionType> like IForwardErrorCorrection if signalForwardErrorCorrectionType != "" {
            @display("p=72,168");
        } // TODO
}