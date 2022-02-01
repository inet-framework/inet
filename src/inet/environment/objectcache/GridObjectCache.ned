//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.objectcache;

import inet.environment.contract.IObjectCache;

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
        double cellSizeX @unit(m) = default(nan m); // NaN means use cell count
        double cellSizeY @unit(m) = default(nan m); // NaN means use cell count
        double cellSizeZ @unit(m) = default(nan m); // NaN means use cell count
        int cellCountX = default(10); // divide space to so many cells
        int cellCountY = default(10); // divide space to so many cells
        int cellCountZ = default(10); // divide space to so many cells
        @display("i=block/table2");
        @class(GridObjectCache);
}

