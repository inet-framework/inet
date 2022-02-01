//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.bitlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IAnalogModel;

module LayeredDimensionalAnalogModel like IAnalogModel
{
    parameters:
        @display("i=block/tunnel");
        @class(LayeredDimensionalAnalogModel);
}

