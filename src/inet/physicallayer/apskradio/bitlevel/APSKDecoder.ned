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

package inet.physicallayer.apskradio.bitlevel;

import inet.physicallayer.contract.bitlevel.IDecoder;
import inet.physicallayer.contract.bitlevel.IFECCoder;
import inet.physicallayer.contract.bitlevel.IInterleaver;
import inet.physicallayer.contract.bitlevel.IScrambler;

//
// This module is part of a simple hypothetical layered receiver. It computes
// the packet domain representation from the bit domain representation by applying
// the configured descrambling, forward errror correction decoding, and deinterleaving.
//
// See also ~APSKEncoder, ~APSKLayeredTransmitter.
//
module APSKDecoder like IDecoder
{
    parameters:
        string deinterleaverType = default("");
        string fecType = default("");
        string descramblerType = default("");
        @display("i=block/blackboard");
        @class(APSKDecoder);

    submodules:
        deinterleaver: <deinterleaverType> like IInterleaver if deinterleaverType != "" {
            @display("p=100,50");
        }
        fecDecoder: <fecType> like IFECCoder if fecType != "" {
            @display("p=100,150");
        }
        descrambler: <descramblerType> like IScrambler if descramblerType != "" {
            @display("p=100,250");
        }
}
