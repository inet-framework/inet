//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

//
// Used in 802.11 infrastructure mode: handles management frames for a station (STA).
// Relies on the MAC layer (~Ieee80211Mac) for reception and transmission of frames.
//
// This variant is a very simplistic: the AP address and channel is preconfigured,
// there is no scanning, authentication or association. Only data frames are
// understood or sent at any time.
//
// This module never switches channels, that is, it will operate on the channel
// the physical layer is configured for (see channelNumber in ~Ieee80211Radio).
//
simple Ieee80211MgmtStaSimplified like IIeee80211Mgmt
{
    parameters:
        string accessPointAddress; // MAC address of associate AP
        string interfaceTableModule;
        string mibModule;
        string macModule;               // The path to the MAC module
        @display("i=block/cogwheel");
    gates:
        input macIn @labels(Ieee80211MacHeader);
        output macOut @labels(Ieee80211MacHeader);
        input agentIn @loose;   // unused dummy to statisfy the IIeee80211Mgmt interface
        output agentOut @loose; // unused dummy to statisfy the IIeee80211Mgmt interface
}

