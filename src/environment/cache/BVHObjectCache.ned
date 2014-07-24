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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.environment.cache;

module BVHObjectCache like IObjectCache
{
    parameters:
        string physicalEnvironmentModule = default("^");
        int leafCapacity = default(1);
        string axisOrder = default("XYZ"); //  "X", "XY", "ZYZ", "ZZXZZYXYX", etc are all allowed combinations
        @class(BVHObjectCache);
}
