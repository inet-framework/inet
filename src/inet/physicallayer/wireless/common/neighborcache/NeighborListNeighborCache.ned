//
// Copyright (C) 2020 OpenSim Ltd.
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

package inet.physicallayer.wireless.common.neighborcache;

import inet.physicallayer.wireless.common.contract.packetlevel.INeighborCache;

//
// This neighbor cache model maintains a separate periodically updated neighbor
// list for each radio.
//
module NeighborListNeighborCache like INeighborCache
{
    parameters:
        string radioMediumModule = default("^");
        double range @unit(m);
        double refillPeriod @unit(s);
        @display("i=block/table2");
        @class(NeighborListNeighborCache);
}

