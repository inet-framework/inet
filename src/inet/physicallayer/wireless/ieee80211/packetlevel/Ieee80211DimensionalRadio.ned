//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// This radio model uses dimensional transmission power (that changes over time
// and/or frequency) in the analog representation. It must be used in
// conjunction with the ~Ieee80211DimensionalRadioMedium model.
//
// @see ~Ieee80211DimensionalRadioMedium, ~Ieee80211Transmitter,
// ~Ieee80211Receiver, ~DimensionalAnalogModel.
//
module Ieee80211DimensionalRadio extends Ieee80211Radio
{
    parameters:
        transmitter.typename = default("Ieee80211Transmitter");
        transmitter.analogModel.typename = default("DimensionalTransmitterAnalogModel");
        receiver.typename = default("Ieee80211Receiver");
        receiver.analogModel.typename = default("DimensionalReceiverAnalogModel");
}

