//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

module DimensionalRadioMedium extends RadioMedium
{
    parameters:
        analogModel.typename = default("DimensionalAnalogModel");
        backgroundNoise.typename = default("IsotropicDimensionalBackgroundNoise");
}

