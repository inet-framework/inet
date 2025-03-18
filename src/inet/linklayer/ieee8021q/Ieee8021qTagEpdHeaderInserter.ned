//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFlowBase;

//
// Inserts IEEE 802.1Q VLAN tag headers with EtherType/Protocol Discrimination (EPD)
// into packets. Supports both Customer VLAN (C-VLAN, TPID 0x8100) and Service VLAN
// (S-VLAN, TPID 0x88A8) tag types. Sets VLAN ID, Priority Code Point (PCP), and
// Drop Eligible Indicator (DEI) fields based on packet tags or default values.
// Determines the TypeOrLength field based on the protocol (packet length for LLC
// or EtherType for other protocols). Used in VLAN-aware network devices to enable
// traffic prioritization and VLAN segmentation in Ethernet networks.
//
simple Ieee8021qTagEpdHeaderInserter extends PacketFlowBase like IIeee8021qTagEpdHeaderInserter
{
    parameters:
        string vlanTagType @enum("c","s") = default("c");
        string nextProtocol = default("ethernetmac");
        int defaultVlanId = default(-1);
        int defaultPcp = default(-1);
        int defaultUserPriority = default(-1);
        int defaultDropEligible = default(-1); // <0: unspecified, 0: false, >0: true
        @class(Ieee8021qTagEpdHeaderInserter);
        @display("i=block/inserter");
}

