//
// Copyright (C) 2013 OpenSim Ltd.
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

package inet.physicallayer.base.packetlevel;

import inet.physicallayer.contract.packetlevel.IPropagation;

//
// This module servces as a base module for propagation models.
//
module PropagationBase like IPropagation
{
    parameters:
        double propagationSpeed @unit(mps) = default(299792458mps); // the theoretical propagation speed of signals on the medium (used by path loss models)
        @display("i=block/timer");
}

