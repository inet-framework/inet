//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.backgroundnoise;

import inet.physicallayer.wireless.common.contract.packetlevel.IBackgroundNoise;

//
// This background noise model describes noise that does not change over space,
// time and frequency. It produces scalar noise signals that can be further used
// in scalar computations.
//
module IsotropicScalarBackgroundNoise like IBackgroundNoise
{
    parameters:
        double power @unit(dBm); // noise power
        double bandwidth @unit(Hz) = default(nan Hz);
        @class(IsotropicScalarBackgroundNoise);
        @display("i=block/mac");
}

