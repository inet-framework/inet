//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.common.analogmodel.scalar;

import inet.common.Module;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitterAnalogModel;

//
// Transmitter analog model using scalar representation for wireless communications.
// Models transmission by representing signal power as a single scalar value
// rather than as a function of time and frequency. Provides a simpler and more
// computationally efficient alternative to dimensional models when detailed
// time-frequency characteristics are not needed.
//
module ScalarTransmitterAnalogModel extends Module like ITransmitterAnalogModel
{
    parameters:
        double power @unit(W) = default(nan W); // Default power
        double centerFrequency @unit(Hz) = default(nan Hz); // Default center frequency
        double bandwidth @unit(Hz) = default(nan Hz); // Default bandwidth
        @class(ScalarTransmitterAnalogModel);
        @display("i=block/tunnel");
}
