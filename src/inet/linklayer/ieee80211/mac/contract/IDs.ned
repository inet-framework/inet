//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 Distribution Service (DS) implementations that determine 
// the fate of correctly received frames. The DS examines frame addressing information 
// to decide whether frames should be delivered to higher layers, forwarded to other 
// stations, or discarded. Essential for implementing access point functionality where 
// frames may need to be relayed between stations or between the wireless network and 
// the distribution system.
//
// @see ~Ds
//
moduleinterface IDs
{
    parameters:
        @display("i=block/routing");
}
