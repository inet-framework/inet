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

package inet.physicallayer.radio.generic;

module QuadTreeNeighborCache like INeighborCache
{
    parameters:
        int maxNumOfPointsPerQuadrant = default(2);
        double range @unit(m) = default(250m);
        double refillPeriod @unit(s) = default(1s);
        double maxSpeed @unit(mps) = default(8mps);
        double constraintAreaMaxX @unit(m) = default(600m); // max x position of the constraint area, unconstrained by default (positive infinity)
        double constraintAreaMaxY @unit(m) = default(600m); // max y position of the constraint area, unconstrained by default (positive infinity)
        double constraintAreaMinX @unit(m) = default(0m); // min x position of the constraint area, unconstrained by default (negative infinity)
        double constraintAreaMinY @unit(m) = default(0m); // min y position of the constraint area, unconstrained by default (negative infinity)
        bool strictQueries = default(false);
        bool isStaticNetwork = default(false);
        @class(QuadTreeNeighborCache);
}
