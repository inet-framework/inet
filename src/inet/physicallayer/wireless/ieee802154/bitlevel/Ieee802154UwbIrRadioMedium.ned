//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee802154.bitlevel;

import inet.physicallayer.wireless.common.medium.RadioMedium;

module Ieee802154UwbIrRadioMedium extends RadioMedium
{
    parameters:
        signalAnalogRepresentation = default("dimensional");
        backgroundNoise.power = default(-110dBm);
        backgroundNoise.dimensions = default("time");
        mediumLimitCache.centerFrequency = default(2.4GHz);
        mediumLimitCache.minReceptionPower = default(-85dBm);
        mediumLimitCache.minInterferencePower = default(-110dBm);
}

