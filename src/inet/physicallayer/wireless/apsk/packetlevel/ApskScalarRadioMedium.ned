//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.packetlevel;

import inet.physicallayer.wireless.common.medium.RadioMedium;

//
// This radio medium model provides a hypothetical radio that simply uses one of
// the  well-known modulations without utilizing other techiques such as forward
// error correction, interleaving, spreading, etc. It must be used in
// conjunction with the ~ApskScalarRadio model.
//
// @see ~ApskScalarRadio, ~ScalarAnalogModel, ~ApskScalarTransmitter,
// ~ApskScalarReceiver.
//
module ApskScalarRadioMedium extends RadioMedium
{
    parameters:
        analogModel.typename = default("ScalarAnalogModel");
        backgroundNoise.typename = default("IsotropicScalarBackgroundNoise");
}

