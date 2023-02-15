//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// This radio medium model uses dimensional transmission power (that changes
// over time and/or frequency) in the analog representation. It must be used in
// conjunction with the dimensional analog model in radios.
//
// @see ~DimensionalMediumAnalogModel
//
module Ieee80211DimensionalRadioMedium extends Ieee80211RadioMedium
{
    parameters:
        signalAnalogRepresentation = default("dimensional");
}

