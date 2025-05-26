//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mgmt;

import inet.common.SimpleModule;

//
// Used in 802.11 infrastructure mode for a station (STA).
//
// This management module variant does not implement scanning, nor does
// it send or expect to receive any management frames. It simply treats the
// station as associated to the configured AP all the time.
//
simple Ieee80211MgmtStaSimplified extends SimpleModule like IIeee80211Mgmt
{
    parameters:
        @class(Ieee80211MgmtStaSimplified);
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
