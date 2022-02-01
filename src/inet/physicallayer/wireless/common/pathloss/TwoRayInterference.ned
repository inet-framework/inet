//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

module TwoRayInterference like IPathLoss
{
    parameters:
        @class(TwoRayInterference);
        @display("i=block/control");
        double epsilon_r = default(1.02);
        string polarization = default("horizontal");
}

