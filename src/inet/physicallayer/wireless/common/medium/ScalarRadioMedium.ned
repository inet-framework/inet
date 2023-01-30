//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211RadioMedium;

module ScalarRadioMedium extends Ieee80211RadioMedium
{
    parameters:
        analogModel.typename = default("ScalarAnalogModel");
        backgroundNoise.typename = default("IsotropicScalarBackgroundNoise");
}

