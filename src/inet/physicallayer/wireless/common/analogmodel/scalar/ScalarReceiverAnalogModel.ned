//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.common.analogmodel.scalar;

import inet.common.SimpleModule;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiverAnalogModel;

//
// Receiver analog model using scalar representation for wireless communications.
// Models reception by representing signal power as a single scalar value rather
// than as a function of time and frequency. Determines whether reception is
// possible by comparing received signal power with the receiver's sensitivity.
// Provides a simpler and more computationally efficient alternative to
// dimensional models.
//
simple ScalarReceiverAnalogModel extends SimpleModule like IReceiverAnalogModel
{
    parameters:
        @class(ScalarReceiverAnalogModel);
        double defaultCenterFrequency @unit(Hz) = default(nan Hz);
        double defaultBandwidth @unit(Hz) = default(nan Hz);
        double defaultSensitivity @unit(dBm) = default(nan dBm);     // Reception is not possible if the signal power is below sensitivity (idle or busy states)
        @display("i=block/tunnel");
}
