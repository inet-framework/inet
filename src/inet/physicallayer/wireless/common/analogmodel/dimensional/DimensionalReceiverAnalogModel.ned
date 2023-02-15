//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.dimensional;

import inet.physicallayer.wireless.common.contract.packetlevel.IReceiverAnalogModel;

module DimensionalReceiverAnalogModel like IReceiverAnalogModel
{
    parameters:
        double centerFrequency @unit(Hz) = default(nan Hz);
        double bandwidth @unit(Hz) = default(nan Hz);
        double energyDetection @unit(dBm) = default(nan dBm); // no signal is detected at all below this reception power threshold (idle state)
        double sensitivity @unit(dBm) = default(nan dBm);     // reception is not possible if the signal power is below sensitivity (idle or busy states)
        @class(DimensionalReceiverAnalogModel);
        @display("i=block/tunnel");
}
