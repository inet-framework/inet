//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.radio.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitterAnalogModel;

module ScalarTransmitterAnalogModel like ITransmitterAnalogModel
{
    parameters:
        double power @unit(W);
        double centerFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        @class(ScalarTransmitterAnalogModel);
        @display("i=block/tunnel");
}
