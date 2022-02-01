//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

module TwoRayGroundReflection extends FreeSpacePathLoss
{
    parameters:
        @class(TwoRayGroundReflection);
        string physicalEnvironmentModule = default("physicalEnvironment"); // the path of the environment model relative to the network
}

