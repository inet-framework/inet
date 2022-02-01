//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.obstacleloss;

import inet.physicallayer.wireless.common.base.packetlevel.TracingObstacleLossBase;

//
// This obstacle loss model determines the power loss by computing the accurate
// dielectric and reflection loss along the straight path considering the shape,
// the position, the orientation, and the material of obstructing physical
// objects.
//
module DielectricObstacleLoss extends TracingObstacleLossBase
{
    parameters:
        bool enableDielectricLoss = default(true); // enables/disables material dielectric loss
        bool enableReflectionLoss = default(true); // enables/disables surface reflection loss
        @class(DielectricObstacleLoss);
}

