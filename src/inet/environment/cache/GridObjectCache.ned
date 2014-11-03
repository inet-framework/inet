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

package inet.environment.cache;

//
// This object cache model stores physical objects in a spatial grid. Each cell
// maintains a list of intersecting physical objects. The grid is aligned with
// the coordinate axes and it has a configurable cell size in all dimensions.
// The cell size parameters take precedence over the cell count parameters.
//
module GridObjectCache like IObjectCache
{
    parameters:
        string physicalEnvironmentModule = default("^");
        double cellSizeX @unit(m) = default(0m/0); // NaN means use cell count
        double cellSizeY @unit(m) = default(0m/0); // NaN means use cell count
        double cellSizeZ @unit(m) = default(0m/0); // NaN means use cell count
        int cellCountX = default(10); // divide space to so many cells
        int cellCountY = default(10); // divide space to so many cells
        int cellCountZ = default(10); // divide space to so many cells
        @display("i=block/cogwheel");
        @class(GridObjectCache);
}
