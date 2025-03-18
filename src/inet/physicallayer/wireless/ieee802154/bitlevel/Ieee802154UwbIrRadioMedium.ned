//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee802154.bitlevel;

import inet.physicallayer.wireless.common.medium.RadioMedium;

//
// Radio medium model for IEEE 802.15.4A Ultra-Wideband Impulse Radio (UWB-IR)
// communication. Provides a specialized configuration of the generic radio medium
// with parameters optimized for UWB-IR, including dimensional signal representation,
// appropriate noise levels, and frequency settings. Designed to work with
// Ieee802154UwbIrRadio modules for simulating low-power, short-range wireless
// communication in applications like wireless sensor networks and IoT devices.
//
module Ieee802154UwbIrRadioMedium extends RadioMedium
{
    parameters:
        signalAnalogRepresentation = default("dimensional");
        backgroundNoise.power = default(-110dBm);
        mediumLimitCache.centerFrequency = default(2.4GHz);
        mediumLimitCache.minReceptionPower = default(-85dBm);
        mediumLimitCache.minInterferencePower = default(-110dBm);
}

