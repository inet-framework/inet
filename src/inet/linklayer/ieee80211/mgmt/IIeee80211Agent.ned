//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Defines the interface for IEEE 802.11 agent modules used in infrastructure mode. 
// The agent acts as a decision-making entity that controls channel scanning, association, 
// and handovers by sending commands to the management module (Ieee80211Mgmt). It determines 
// when to scan for access points, which access point to associate with based on signal 
// strength or other criteria, when to perform handovers between access points, and handles 
// authentication and association processes. Implementations of this interface (such as 
// Ieee80211AgentSta) provide specific behaviors for different network roles and configurations.
//
// @see ~Ieee80211AgentSta, ~Ieee80211MgmtSta
//
moduleinterface IIeee80211Agent
{
    parameters:
        @display("i=block/cogwheel");
    gates:
        input mgmtIn;
        output mgmtOut;
}
