//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.linklayer.ieee80211.mac.contract.IDs;

//
// Implements the DS (Distribution Service) for IEEE 802.11, which is
// responsible for distributing correctly received frames to the higher
// layer, to the wireless LAN, etc.
//
// This is the default implementation of ~IDs.
//
simple Ds like IDs
{
    parameters:
        string mibModule;
        @display("i=block/routing");
        @signal[packetDropped];
}

