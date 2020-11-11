//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

package inet.physicallayer.wireless.apsk.bitlevel;

import inet.physicallayer.wireless.common.contract.bitlevel.IEncoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IFecCoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IInterleaver;
import inet.physicallayer.wireless.common.contract.bitlevel.IScrambler;

//
// This module is part of a simple hypothetical layered transmitter. It computes
// the bit domain representation from the packet domain representation by applying
// the configured scrambling, forward errror correction encoding, and interleaving.
//
// @see ~ApskDecoder, ~ApskLayeredReceiver.
//
module ApskEncoder like IEncoder
{
    parameters:
        @display("i=block/blackboard");
        @class(ApskEncoder);

    submodules:
        scrambler: <default("")> like IScrambler if typename != "" {
            @display("p=100,100");
        }
        fecEncoder: <default("")> like IFecCoder if typename != "" {
            @display("p=100,200");
        }
        interleaver: <default("")> like IInterleaver if typename != "" {
            @display("p=100,300");
        }
}

