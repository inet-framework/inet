//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.scalar;

import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitterAnalogModel;

module ScalarTransmitterAnalogModel like ITransmitterAnalogModel
{
    parameters:
        double power @unit(W) = default(nan W); // default power
        double centerFrequency @unit(Hz) = default(nan Hz); // default center frequency
        double bandwidth @unit(Hz) = default(nan Hz); // default bandwidth
        @class(ScalarTransmitterAnalogModel);
        @display("i=block/tunnel");
}
