//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandReceiverBase;

//
// This receiver model receives a transmission succesfully if the minimum of the
// signal to noise and interference ratio over the duration of the reception is
// sufficiently high. It uses the error model to compute the error rate based on
// this value and the used modulation.
//
// @see ~ApskScalarTransmitter, ~ApskScalarRadio, ~ApskScalarRadioMedium,
// ~ScalarAnalogModel.
//
module ApskScalarReceiver extends NarrowbandReceiverBase
{
    parameters:
        modulation = default("BPSK");
        errorModel.typename = default("ApskErrorModel");
        @class(ApskScalarReceiver);
}

