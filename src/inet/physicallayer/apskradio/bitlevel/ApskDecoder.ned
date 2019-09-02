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
import inet.physicallayer.contract.bitlevel.IFecCoder;
import inet.physicallayer.contract.bitlevel.IInterleaver;
import inet.physicallayer.contract.bitlevel.IScrambler;

//
// This module is part of a simple hypothetical layered receiver. It computes
// the packet domain representation from the bit domain representation by applying
// the configured descrambling, forward errror correction decoding, and deinterleaving.
//
// @see ~ApskEncoder, ~ApskLayeredTransmitter.
//
module ApskDecoder like IDecoder
{
    parameters:
        @display("i=block/blackboard");
        @class(ApskDecoder);

    submodules:
        deinterleaver: <default("")> like IInterleaver if typename != "" {
            @display("p=100,100");
        }
        fecDecoder: <default("")> like IFecCoder if typename != "" {
            @display("p=100,200");
        }
        descrambler: <default("")> like IScrambler if typename != "" {
            @display("p=100,300");
        }
}

