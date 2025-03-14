//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Provides a simplified station (STA) management module for IEEE 802.11 infrastructure 
// mode networks. Unlike the full ~Ieee80211MgmtSta implementation, this variant completely 
// bypasses the standard management procedures by treating the station as permanently 
// associated with the configured access point (AP) without requiring any scanning, 
// authentication, or association processes. It drops all management frames it receives 
// and does not generate any management frames of its own.
//
// During initialization, this module automatically configures itself as associated with 
// the AP specified by the accessPointAddress parameter. It also registers itself with 
// the AP's management module so that the AP recognizes the station as associated. This 
// allows immediate data frame exchange without the overhead of the standard connection 
// establishment process.
//
// This module is useful for simulations where the focus is on data transmission rather 
// than management procedures, or when simplified network setup is desired.
//
// @see ~IIeee80211Mgmt, ~Ieee80211MgmtSta, ~Ieee80211MgmtAp, ~Ieee80211MgmtApSimplified
//
simple Ieee80211MgmtStaSimplified like IIeee80211Mgmt
{
    parameters:
        string accessPointAddress; // MAC address of associated AP
        string interfaceTableModule;
        string mibModule;
        string macModule;               // The path to the MAC module
        @display("i=block/cogwheel");
    gates:
        input macIn @labels(Ieee80211MacHeader);
        output macOut @labels(Ieee80211MacHeader);
        input agentIn @loose;   // Unused dummy to satisfy the IIeee80211Mgmt interface
        output agentOut @loose; // Unused dummy to satisfy the IIeee80211Mgmt interface
}
