//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// This radio model uses ideal analog representation.
// It must be used in conjunction with the ~UnitDiskRadioMedium model.
//
// @see ~UnitDiskRadioMedium, ~Ieee80211UnitDiskTransmitter,
// ~UnitDiskReceiver, ~UnitDiskAnalogModel.
//
module Ieee80211UnitDiskRadio extends Ieee80211Radio
{
    parameters:
        transmitter.typename = default("Ieee80211UnitDiskTransmitter");
        receiver.typename = default("Ieee80211UnitDiskReceiver");
        @class(Radio);
}

