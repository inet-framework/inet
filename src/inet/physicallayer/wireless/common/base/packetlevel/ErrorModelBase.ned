//
// Copyright (C) 2013 OpenSim Ltd.
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

package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IErrorModel;

//
// This module servces as the base module for error models.
//
module ErrorModelBase like IErrorModel
{
    parameters:
        string corruptionMode @enum("packet", "chunk", "byte", "bit") = default("packet");
        string snirMode @unit("min", "mean") = default("min");
        double snirOffset @unit(dB) = default(0dB); // offset the calculated SNIR before further computing the PER, BER, SER or corrupt bits and symbols
                                                    // this can be used, for example, to represents cross symbol interference due to OFDM lobe overlapping, symbol frequency/time offset mismatch, and symbol frequency/time size mismatch
        @display("i=block/broadcast");
        @class(ErrorModelBase);
}

