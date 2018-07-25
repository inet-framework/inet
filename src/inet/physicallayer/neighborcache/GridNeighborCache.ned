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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.physicallayer.neighborcache;

import inet.physicallayer.contract.packetlevel.INeighborCache;

//
// This neighbor cache model organizes radios in a 3 dimensional grid with
// constant cell size and updates periodically.
//
module GridNeighborCache like INeighborCache
{
    parameters:
        string radioMediumModule = default("^");
        double cellSizeX @unit(m) = default(nan m); // NaN means use cell count
        double cellSizeY @unit(m) = default(nan m); // NaN means use cell count
        double cellSizeZ @unit(m) = default(nan m); // NaN means use cell count
        int cellCountX = default(10); // divide space to so many cells
        int cellCountY = default(10); // divide space to so many cells
        int cellCountZ = default(10); // divide space to so many cells
        double refillPeriod @unit(s) = default(1s);
        @display("i=block/table2");
        @class(GridNeighborCache);
}

