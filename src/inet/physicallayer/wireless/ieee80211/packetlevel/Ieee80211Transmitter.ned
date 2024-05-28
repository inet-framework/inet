//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandTransmitterBase;

//
// This transmitter model produces IEEE 802.11 transmissions.
//
// @see ~Ieee80211Receiver, ~Ieee80211Radio, ~Ieee80211RadioMedium.
//
module Ieee80211Transmitter extends NarrowbandTransmitterBase
{
    parameters:
        string opMode @enum("a","b","g(erp)","g(mixed)","n(mixed-2.4Ghz)","p","ac");
        string bandName @enum("2.4 GHz","5 GHz","5 GHz (20 MHz)","5 GHz (40 MHz)","5 GHz (80 MHz)","5 GHz (160 MHz)","5.9 GHz");
        int channelNumber;
        modulation = default("BPSK"); // TODO: This is simply wrong
        @class(Ieee80211Transmitter);
}
