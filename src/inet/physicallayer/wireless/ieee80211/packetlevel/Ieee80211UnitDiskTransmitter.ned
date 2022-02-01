//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

//
// @see ~Ieee80211UnitDiskReceiver, ~Ieee80211UnitDiskRadio, ~UnitDiskRadioMedium.
//
module Ieee80211UnitDiskTransmitter extends Ieee80211TransmitterBase
{
    parameters:
        double communicationRange @unit(m);                              // the range where the reception of transmissions produced by this transmitter is possible
        double interferenceRange @unit(m) = default(communicationRange); // the range where transmissions produced by this transmitter interferes with other transmissions
        double detectionRange @unit(m) = default(interferenceRange);     // the range where transmissions produced by this transmitter can be detected at all
        @class(Ieee80211UnitDiskTransmitter);
}

