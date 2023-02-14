//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IAnalogModel;

//
// This analog model computes with scalar analog power representation. In other
// words, the signal power does not change over time or frequency, except for
// the boundaries of the signal.
//
module ScalarAnalogModel like IAnalogModel
{
    parameters:
        bool ignorePartialInterference = default(false); // enabled means interference only partially overlapping with the signal being received is ignored, otherwise the simulation stops with an error
        @display("i=block/tunnel");
        @class(ScalarAnalogModel);
}

