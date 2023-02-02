//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.IReceiverAnalogModel;

module ScalarReceiverAnalogModel like IReceiverAnalogModel
{
    parameters:
        double centerFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        double energyDetection @unit(dBm); // no signal is detected at all below this reception power threshold (idle state)
        double sensitivity @unit(dBm);     // reception is not possible if the signal power is below sensitivity (idle or busy states)
        @class(ScalarReceiverAnalogModel);
        //@display("i=block/tunnel");
}
