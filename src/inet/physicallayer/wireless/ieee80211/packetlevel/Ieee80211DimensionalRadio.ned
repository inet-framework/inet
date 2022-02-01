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
// @see ~Ieee80211DimensionalRadioMedium, ~Ieee80211DimensionalTransmitter,
// ~Ieee80211DimensionalReceiver, ~DimensionalAnalogModel.
//
module Ieee80211DimensionalRadio extends Ieee80211Radio
{
    parameters:
        transmitter.typename = default("Ieee80211DimensionalTransmitter");
        receiver.typename = default("Ieee80211DimensionalReceiver");
}

