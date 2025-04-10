//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

import inet.common.SimpleModule;

//
// Used in 802.11 infrastructure mode in an access point (AP).
//
// This management module variant does not send or expect to receive any
// management frames -- it simply treats all stations as associated all the time.
//
simple Ieee80211MgmtApSimplified extends SimpleModule like IIeee80211Mgmt
{
    parameters:
        @class(Ieee80211MgmtApSimplified);
        string interfaceTableModule;
        string mibModule;
        string macModule;               // The path to the MAC module
        string ssid = default("SSID");
        @display("i=block/cogwheel");
    gates:
        input macIn @labels(Ieee80211MacHeader); // From ~Ieee80211Mac
        output macOut @labels(Ieee80211MacHeader); // To ~Ieee80211Mac
        input agentIn @loose;   // Unused dummy to satisfy the IIeee80211Mgmt interface
        output agentOut @loose; // Unused dummy to satisfy the IIeee80211Mgmt interface
}
