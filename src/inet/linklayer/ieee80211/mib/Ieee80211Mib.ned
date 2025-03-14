//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mib;

//
// Implements the Management Information Base (MIB) for IEEE 802.11 networks as defined in the 
// IEEE 802.11 standard. This module stores configuration and state information related to the 
// IEEE 802.11 MAC layer, including the station's MAC address, operation mode (infrastructure, 
// ad-hoc, or mesh), QoS capability, BSS (Basic Service Set) information (SSID and BSSID), 
// station type (access point or station), association status, and information about connected 
// stations (for access points). Other modules in the IEEE 802.11 implementation access this 
// information to make decisions about frame handling, association, authentication, and other 
// MAC layer operations.
//
simple Ieee80211Mib
{
    parameters:
        @display("i=block/table");
}
