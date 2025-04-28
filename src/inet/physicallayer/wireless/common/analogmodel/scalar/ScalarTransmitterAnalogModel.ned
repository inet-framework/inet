//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.analogmodel.scalar;

import inet.common.SimpleModule;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitterAnalogModel;

module ScalarTransmitterAnalogModel extends SimpleModule like ITransmitterAnalogModel
{
    parameters:
        double power @unit(W) = default(nan W); // Default power
        double centerFrequency @unit(Hz) = default(nan Hz); // Default center frequency
        double bandwidth @unit(Hz) = default(nan Hz); // Default bandwidth
        @class(ScalarTransmitterAnalogModel);
        @display("i=block/tunnel");
}
