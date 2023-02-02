//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitterAnalogModel;

module DimensionalTransmitterAnalogModel like ITransmitterAnalogModel
{
    parameters:
//        double power @unit(W);
//        double centerFrequency @unit(Hz);
//        double bandwidth @unit(Hz);
        @class(DimensionalTransmitterAnalogModel);
        @display("i=block/tunnel");
}
