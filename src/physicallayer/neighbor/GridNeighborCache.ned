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

package inet.physicallayer.neighbor;

import inet.physicallayer.contract.INeighborCache;

module GridNeighborCache like INeighborCache
{
    parameters:
        double cellSizeX @unit(m);
        double cellSizeY @unit(m);
        double cellSizeZ @unit(m);

        double constraintAreaMinX @unit(m); // min x position of the constraint area, unconstrained by default (negative infinity)
        double constraintAreaMinY @unit(m); // min y position of the constraint area, unconstrained by default (negative infinity)
        double constraintAreaMinZ @unit(m); // min z position of the constraint area, unconstrained by default (negative infinity)
        double constraintAreaMaxX @unit(m); // max x position of the constraint area, unconstrained by default (positive infinity)
        double constraintAreaMaxY @unit(m); // max y position of the constraint area, unconstrained by default (positive infinity)
        double constraintAreaMaxZ @unit(m); // max z position of the constraint area, unconstrained by default (positive infinity)

        double range @unit(m);
        double refillPeriod @unit(s);

        @class(GridNeighborCache);
}
