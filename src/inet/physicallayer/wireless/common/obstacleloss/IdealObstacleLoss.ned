//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.obstacleloss;

import inet.physicallayer.wireless.common.base.packetlevel.TracingObstacleLossBase;

//
// This obstacle loss model determines power loss by checking if there is any
// obstructing physical object along the straight propagation path. The result
// is either total power loss if there was such an object or no loss at all if
// there wasn't.
//
module IdealObstacleLoss extends TracingObstacleLossBase
{
    parameters:
        @class(IdealObstacleLoss);
}

