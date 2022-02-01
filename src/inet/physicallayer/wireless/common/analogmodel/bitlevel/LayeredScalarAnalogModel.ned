//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.bitlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IAnalogModel;

module LayeredScalarAnalogModel like IAnalogModel
{
    parameters:
        bool ignorePartialInterference = default(false); // enabled means interference only partially overlapping with the signal being received is ignored, otherwise the simulation stops with an error
        @display("i=block/tunnel");
        @class(LayeredScalarAnalogModel);
}

