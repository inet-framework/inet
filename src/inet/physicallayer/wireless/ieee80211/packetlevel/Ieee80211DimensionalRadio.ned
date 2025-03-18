//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// Radio model for IEEE 802.11 networks that uses dimensional analysis to
// represent signals as functions of time and frequency. Provides more accurate
// modeling of signal propagation, interference, and reception compared to scalar
// and unit disk models. Supports configurable time-varying and frequency-varying
// gain patterns with various interpolation methods, enabling realistic
// simulation of wireless transmissions in complex environments.
//
module Ieee80211DimensionalRadio extends Ieee80211Radio
{
    parameters:
        signalAnalogRepresentation = default("dimensional");
}
