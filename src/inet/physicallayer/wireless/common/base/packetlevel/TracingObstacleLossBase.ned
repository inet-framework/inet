//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IObstacleLoss;

//
// This module servces as a base module for tracing obstacle loss models.
//
module TracingObstacleLossBase like IObstacleLoss
{
    parameters:
        string physicalEnvironmentModule = default("physicalEnvironment"); // module path of the physical environment model
        @display("i=block/control");
        @signal[obstaclePenetrated];
}

