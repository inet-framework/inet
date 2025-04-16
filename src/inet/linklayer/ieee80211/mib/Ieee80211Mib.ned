//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mib;

//
// Management Information Base for IEEE 802.11 networks. Stores configuration and
// state information including network mode (infrastructure, ad-hoc, mesh),
// station type (access point or station), authentication and association status,
// and BSS data (SSID, BSSID). Used by various IEEE 802.11 modules to maintain
// protocol state and control frame addressing.
//
simple Ieee80211Mib
{
    parameters:
        @display("i=block/table");
}

