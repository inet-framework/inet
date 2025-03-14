//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.linklayer.ieee80211.mac.contract.IDs;

//
// Distribution Service (DS) component for IEEE 802.11 that determines the appropriate 
// handling of correctly received frames. Makes forwarding decisions based on frame 
// addressing information, determining whether frames should be delivered to higher layers, 
// forwarded to other stations, or discarded. Acts as a central routing mechanism within 
// the MAC layer, particularly important in access point operation.
//
// @see ~IDs
//
simple Ds like IDs
{
    parameters:
        string mibModule;
        @display("i=block/routing");
        @signal[packetDropped];
}
