//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Implements the management functionality for IEEE 802.11 ad-hoc mode networks. In ad-hoc mode 
// (also known as Independent Basic Service Set or IBSS), stations communicate directly with 
// each other without the need for an access point. This module provides a simplified implementation 
// that focuses on data frame handling while ignoring most management functionality. It never 
// sends management frames (such as beacons, authentication, or association frames) and discards 
// any such frames received. The module sets the MIB mode to INDEPENDENT and does not support 
// any management commands. Distributed beacon generation that would normally occur in real 
// ad-hoc networks is not modeled.
//
// @see ~IIeee80211Mgmt, ~Ieee80211MgmtAp, ~Ieee80211MgmtSta
//
simple Ieee80211MgmtAdhoc like IIeee80211Mgmt
{
    parameters:
        string mibModule;
        string macModule;               // The path to the MAC module
        string interfaceTableModule;   // The path to the InterfaceTable module
        @display("i=block/cogwheel");
    gates:
        input macIn @labels(Ieee80211MacHeader);
        output macOut @labels(Ieee80211MacHeader);
        input agentIn @loose;   // Unused dummy to satisfy the IIeee80211Mgmt interface
        output agentOut @loose; // Unused dummy to satisfy the IIeee80211Mgmt interface
}
