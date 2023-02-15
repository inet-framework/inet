//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// This radio medium model uses scalar transmission power in the analog
// representation. It must be used in conjunction with the analog model
// in radios.
//
// @see ~ScalarMediumAnalogModel
//
module Ieee80211ScalarRadioMedium extends Ieee80211RadioMedium
{
    parameters:
        signalAnalogRepresentation = default("scalar");
}

