//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.IObstacleLoss;

//
// Serves as a base module for tracing obstacle loss models.
//
module TracingObstacleLossBase extends Module like IObstacleLoss
{
    parameters:
        string physicalEnvironmentModule = default("physicalEnvironment"); // Module path of the physical environment model
        @display("i=block/control");
        @signal[obstaclePenetrated];
}

