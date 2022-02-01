//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// This radio medium model uses scalar transmission power in the analog
// representation. It must be used in conjunction with the
// ~Ieee80211ScalarRadio model.
//
// @see ~Ieee80211ScalarRadio, ~Ieee80211ScalarTransmitter,
// ~Ieee80211ScalarReceiver, ~ScalarAnalogModel.
//
module Ieee80211ScalarRadioMedium extends Ieee80211RadioMedium
{
    parameters:
        analogModel.typename = default("ScalarAnalogModel");
        backgroundNoise.typename = default("IsotropicScalarBackgroundNoise");
}

