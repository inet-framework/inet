//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;

module SuiPathLoss like IPathLoss
{
    parameters:
        @class(SuiPathLoss);
        @display("i=block/control");
        double transmitterAntennaHeight @unit(m);
        double receiverAntennaHeight @unit(m);
}

