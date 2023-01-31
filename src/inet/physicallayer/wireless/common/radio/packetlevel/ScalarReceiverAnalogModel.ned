//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.radio.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IReceiverAnalogModel;

module ScalarReceiverAnalogModel like IReceiverAnalogModel
{
    parameters:
        double centerFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        // sensitivity?
        @class(ScalarReceiverAnalogModel);
        //@display("i=block/tunnel");
}
