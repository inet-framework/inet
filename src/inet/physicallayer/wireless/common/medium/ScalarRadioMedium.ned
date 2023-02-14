//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

module ScalarRadioMedium extends RadioMedium
{
    parameters:
        analogModel.typename = default("ScalarAnalogModel");
        backgroundNoise.typename = default("IsotropicScalarBackgroundNoise");
}

