//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

module FreeSpacePathLoss like IPathLoss
{
    parameters:
        double alpha = default(2);
        double systemLoss @unit(dB) = default(0dB);
        @class(FreeSpacePathLoss);
        @display("i=block/control");
}

