//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

import inet.physicallayer.wireless.common.medium.RadioMedium;

//
// This radio medium model is part of thee IEEE 802.11 physical layer model. It
// must be used in conjunction with the ~Ieee80211Radio model or other derived
// models.
//
// This model provides sensible default values for the radio medium parameters
// to be used in IEEE 802.11 simulations.
//
// @see ~Ieee80211ScalarRadioMedium, ~Ieee80211DimensionalRadioMedium.
//
module Ieee80211RadioMedium extends RadioMedium
{
    parameters:
        backgroundNoise.power = default(-110dBm);
        mediumLimitCache.centerFrequency = default(2.4GHz);
        mediumLimitCache.minReceptionPower = default(-85dBm);
        mediumLimitCache.minInterferencePower = default(-110dBm);
}

