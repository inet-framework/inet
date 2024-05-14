//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.dimensional;

import inet.physicallayer.wireless.common.contract.packetlevel.IReceiverAnalogModel;

simple DimensionalReceiverAnalogModel like IReceiverAnalogModel
{
    parameters:
        double defaultCenterFrequency @unit(Hz) = default(nan Hz);
        double defaultBandwidth @unit(Hz) = default(nan Hz);
        double defaultSensitivity @unit(dBm) = default(nan dBm);     // Reception is not possible if the signal power is below the sensitivity (idle or busy states).
        @display("i=block/tunnel");
}
