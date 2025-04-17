//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

import inet.common.SimpleModule;

//
// IEEE 802.11 management module used for ad-hoc mode.
// This implementation never sends control or management frames, and discards
// any such frame received. Distributed beacon generation is not modeled.
//
simple Ieee80211MgmtAdhoc extends SimpleModule like IIeee80211Mgmt
{
    parameters:
        @class(Ieee80211MgmtAdhoc);
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
