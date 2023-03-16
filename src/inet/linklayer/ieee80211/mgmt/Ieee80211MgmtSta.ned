//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Used in 802.11 infrastructure mode: handles management frames for a station (STA).
//
// This module accepts commands such as Scan, Authenticate, Deauthenticate,
// Associate, Reassociate, Deassociate from a connected agent module, and carries
// out the requested action by sending/receiving management frames, switching
// channels, etc.
//
// It implementes both active and passive scanning (where active means sending
// Probe frames). The authentication process is simulated by exchanging dummy
// messages, i.e. there is no actual cryptography or not its detailed simulation
// involved.
//
// Relies on the MAC layer (~Ieee80211Mac) for reception and transmission of frames.
//
simple Ieee80211MgmtSta like IIeee80211Mgmt
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        int numChannels = default(1);  // number of channels to scan
        string mibModule;
        string macModule;               // The path to the MAC module
        @display("i=block/cogwheel");
        @signal[l2Associated](type=inet::NetworkInterface);
        @signal[l2BeaconLost](type=inet::NetworkInterface);
    gates:
        input macIn @labels(Ieee80211MacHeader);
        output macOut @labels(Ieee80211MacHeader);
        input agentIn;   // used for attaching an agent algorithm
        output agentOut; // used for attaching an agent algorithm
}

