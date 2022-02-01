//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.linklayer.ieee80211.mac.contract.IDs;

//
// The default implementation of ~IDs.
//
simple Ds like IDs
{
    parameters:
        string mibModule;
        @display("i=block/routing");
        @signal[packetDropped];
}

