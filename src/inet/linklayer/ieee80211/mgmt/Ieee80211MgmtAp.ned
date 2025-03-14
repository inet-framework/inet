//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Implements the management functionality for an access point (AP) in IEEE 802.11 infrastructure 
// mode networks. This module handles various management frames including authentication, 
// association/disassociation, beacon transmission, and probe requests/responses. It maintains 
// a list of associated stations and their states, periodically broadcasts beacon frames 
// containing network information (SSID, supported rates, etc.), processes authentication 
// and association requests from stations, and relays data frames between associated stations 
// or to/from the distribution system.
//
// The module supports different authentication methods (Open System or WEP) based on the 
// numAuthSteps parameter. It operates on the channel configured for the physical layer 
// (see channelNumber in ~Ieee80211Radio) and does not perform channel switching. It relies 
// on a connected ~Ieee80211Mac module for the actual transmission and reception of frames.
//
// This module expects ~Ieee80211MgmtSta modules to be present in the client stations.
//
// @see ~IIeee80211Mgmt, ~Ieee80211MgmtSta, ~Ieee80211MgmtAdhoc
//
simple Ieee80211MgmtAp like IIeee80211Mgmt
{
    parameters:
        string ssid = default("SSID");
        double beaconInterval @unit(s) = default(100ms);
        int numAuthSteps = default(4); // Use 2 for Open System auth, 4 for WEP
        string interfaceTableModule;
        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value
        string mibModule;
        string macModule;               // The path to the MAC module
        @display("i=block/cogwheel");
        @signal[l2ApAssociated](type="inet::ieee80211::Ieee80211MgmtAp::NotificationInfoSta");
        @signal[l2ApDisassociated](type="inet::ieee80211::Ieee80211MgmtAp::NotificationInfoSta");
    gates:
        input macIn @labels(Ieee80211MacHeader); // From ~Ieee80211Mac
        output macOut @labels(Ieee80211MacHeader); // To ~Ieee80211Mac
        input agentIn @loose;   // Unused dummy to satisfy the IIeee80211Mgmt interface
        output agentOut @loose; // Unused dummy to satisfy the IIeee80211Mgmt interface
}
