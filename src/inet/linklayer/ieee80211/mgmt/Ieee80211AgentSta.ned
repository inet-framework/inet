//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Implements the station (STA) agent functionality for IEEE 802.11 infrastructure mode networks.
// This module controls the entire connection lifecycle including channel scanning, access point
// selection, authentication, association, and handovers between access points. It operates by
// sending management primitives (e.g., ~Ieee80211Prim_ScanRequest) to the management module
// (~Ieee80211MgmtSta) and processing the corresponding confirms.
//
// The agent supports both active scanning (sending probe requests) and passive scanning
// (listening for beacons). When scanning completes, it selects an access point based on
// signal strength, authenticates with it, and then attempts to associate. It also monitors
// the connection quality and can initiate handovers to better access points when necessary.
//
// @see ~IIeee80211Agent, ~Ieee80211MgmtSta
//
simple Ieee80211AgentSta like IIeee80211Agent
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string mibModule;
        bool activeScan = default(true);  // Selects between active and passive scanning
        string channelsToScan = default("");  // List of channel numbers to scan (space delimited); empty means all channels
        double probeDelay @unit(s) = default(100ms); // Delay before sending a probe request during active scanning
        double minChannelTime @unit(s) = default(150ms); // Min interval to spend on a channel during active scanning
        double maxChannelTime @unit(s) = default(300ms); // Channel time for passive scanning, and max channel time for active scanning
        double authenticationTimeout @unit(s) = default(5s); // Timeout for the authentication procedure
        double associationTimeout @unit(s) = default(5s); // Timeout for the association procedure
        double startingTime @unit(s) = default(-1s); // Agent starting time.
        string defaultSsid = default("SSID"); // Default SSID
        @display("i=block/control");
        @signal[sentRequest](type=long); // ENUM! ~Ieee80211PrimRequestCode
        @signal[acceptConfirm](type=long); // ENUM! ~Ieee80211PrimConfirmCode
        @signal[dropConfirm](type=long); // ENUM! ~Ieee80211PrimConfirmCode
        @signal[l2AssociatedNewAp](type=inet::NetworkInterface);
        @signal[l2AssociatedOldAp](type=inet::NetworkInterface);
        @signal[l2Disassociated](type=inet::NetworkInterface);
        @statistic[sentRequest](title="sent requests"; record=count,vector); // ENUM! ~Ieee80211PrimRequestCode
        @statistic[acceptConfirm](title="accepted confirms"; record=count,vector); // ENUM! ~Ieee80211PrimConfirmCode
        @statistic[dropConfirm](title="dropped confirms"; record=count,vector); // ENUM! ~Ieee80211PrimConfirmCode
    gates:
        input mgmtIn;
        output mgmtOut;
}
