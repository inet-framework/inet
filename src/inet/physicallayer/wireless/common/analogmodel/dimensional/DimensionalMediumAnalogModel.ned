//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.dimensional;

import inet.physicallayer.wireless.common.contract.packetlevel.IMediumAnalogModel;

//
// This analog model computes with dimensional analog power representation. In
// other words, the signal power may change over time and/or frequency.
//
module DimensionalMediumAnalogModel like IMediumAnalogModel
{
    parameters:
        bool attenuateWithCenterFrequency = default(true); // True means frequency selectivity will always use the carrier frequency as opposed to using the frequency where the change in power occurs
        @display("i=block/tunnel");
        @class(DimensionalMediumAnalogModel);
}

