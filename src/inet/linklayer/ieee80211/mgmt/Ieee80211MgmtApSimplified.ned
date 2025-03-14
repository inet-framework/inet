//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Provides a simplified access point (AP) management module for IEEE 802.11 infrastructure 
// mode networks. Unlike the full ~Ieee80211MgmtAp implementation, this variant completely 
// bypasses the standard management procedures by treating all stations as permanently 
// associated without requiring any authentication or association processes. It drops all 
// management frames it receives and does not generate any management frames of its own 
// (such as beacons).
//
// This module is useful for simulations where the focus is on data transmission rather 
// than management procedures, or when simplified network setup is desired. It allows 
// stations to immediately start sending data frames without going through the standard 
// connection establishment process, reducing simulation complexity and overhead.
//
// @see ~IIeee80211Mgmt, ~Ieee80211MgmtAp, ~Ieee80211MgmtSta
//
simple Ieee80211MgmtApSimplified like IIeee80211Mgmt
{
    parameters:
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
