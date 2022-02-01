//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.objectcache;

import inet.environment.contract.IObjectCache;

//
// This object cache model organizes closely positioned physical objects into a
// tree data structure.
//
module BvhObjectCache like IObjectCache
{
    parameters:
        string physicalEnvironmentModule = default("^");
        int leafCapacity = default(1); // the upper bound for the number of physical objects per leaf
        string axisOrder = default("XYZ"); //  "X", "XY", "ZYZ", "ZZXZZYXYX", etc are all allowed combinations
        @display("i=block/table2");
        @class(BvhObjectCache);
}

