//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// This radio model uses scalar transmission power in the analog representation.
// It must be used in conjunction with the ~Ieee80211ScalarRadioMedium model.
// @see ~Ieee80211ScalarRadioMedium, ~Ieee80211Transmitter,
// ~Ieee80211Receiver, ~ScalarAnalogModel.
//
module Ieee80211ScalarRadio extends Ieee80211Radio
{
    parameters:
        transmitter.analogModel.typename = default("ScalarTransmitterAnalogModel");
        receiver.analogModel.typename = default("ScalarReceiverAnalogModel");
}

