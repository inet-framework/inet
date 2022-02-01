//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Used in 802.11 infrastructure mode: in a station (STA), this module
// controls channel scanning, association and handovers, by sending commands
// (e.g. ~Ieee80211Prim_ScanRequest) to the management module (~Ieee80211MgmtSta).
//
simple Ieee80211AgentSta like IIeee80211Agent
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string mibModule;
        bool activeScan = default(true);  // selects between active and passive scanning
        string channelsToScan = default("");  // list of channel numbers to scan (space delimited); empty means all channels
        double probeDelay @unit(s) = default(100ms); // delay before sending a probe request during active scanning
        double minChannelTime @unit(s) = default(150ms); // min interval to spend on a channel during active scanning
        double maxChannelTime @unit(s) = default(300ms); // channel time for passive scanning, and max channel time for active scanning
        double authenticationTimeout @unit(s) = default(5s); // timeout for the authentication procedure
        double associationTimeout @unit(s) = default(5s); // timeout for the association procedure
        double startingTime @unit(s) = default(-1s); // agent starting time.
        string defaultSsid = default("SSID"); // default ssid
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

