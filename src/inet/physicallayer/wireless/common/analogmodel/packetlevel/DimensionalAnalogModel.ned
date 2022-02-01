//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IAnalogModel;

//
// This analog model computes with dimensional analog power representation. In
// other words the signal power may change over time and/or frequency.
//
module DimensionalAnalogModel like IAnalogModel
{
    parameters:
        bool attenuateWithCenterFrequency = default(true); // true means frequency selectivity will always use the carrier frequencey as opposed to using the frequency where the change in power occurs
        @display("i=block/tunnel");
        @class(DimensionalAnalogModel);
}

