//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Implements the station (STA) management functionality for IEEE 802.11 networks
// operating in infrastructure mode. This module handles all management aspects
// of a wireless station connecting to and communicating with access points.
//
// The management module is responsible for the complete connection lifecycle:
// - Network discovery through scanning (both passive and active)
// - Authentication with access points
// - Association/reassociation to establish data connectivity
// - Monitoring connection quality and handling roaming
// - Graceful disconnection through deauthentication/disassociation
//
// Key features:
// - Dual scanning modes: passive (listening for beacons) and active (sending probes)
// - Complete state machine implementation for IEEE 802.11 station operation
// - Support for management frame exchange with access points
// - Automatic channel switching during scanning operations
// - Connection quality monitoring with beacon loss detection
// - Interface with an agent module that can initiate management operations
//
// The module processes management commands from the agent (such as scan requests
// or association commands) and translates them into appropriate management frame
// exchanges with access points. It maintains the current state of the connection
// and notifies upper layers about significant events like successful association
// or beacon loss.
//
// The authentication process is simplified and does not implement actual security
// mechanisms, focusing instead on the frame exchange patterns defined in the standard.
//
// @see ~IIeee80211Mgmt, ~Ieee80211MgmtAp, ~Ieee80211Mac
//
simple Ieee80211MgmtSta like IIeee80211Mgmt
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        int numChannels = default(1);  // Number of channels to scan
        string mibModule;
        string macModule;               // The path to the MAC module
        @display("i=block/cogwheel");
        @signal[l2Associated](type=inet::NetworkInterface);
        @signal[l2BeaconLost](type=inet::NetworkInterface);
    gates:
        input macIn @labels(Ieee80211MacHeader);
        output macOut @labels(Ieee80211MacHeader);
        input agentIn;   // Used for attaching an agent algorithm
        output agentOut; // Used for attaching an agent algorithm
}
