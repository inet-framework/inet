//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFlowBase;

//
// Inserts IEEE 802.1Q tag headers with TPID (Tag Protocol Identifier) fields
// into Ethernet frames. Supports both C-VLAN (0x8100) and S-VLAN (0x88A8) tag
// types through the vlanTagType parameter. Adds VLAN ID, priority (PCP), and
// drop eligibility (DEI) fields that enable network segmentation, quality of
// service, and congestion management in Ethernet networks. Used in the
// implementation of Virtual LAN functionality according to the IEEE 802.1Q
// standard.
//
simple Ieee8021qTagTpidHeaderInserter extends PacketFlowBase like IIeee8021qTagTpidHeaderInserter
{
    parameters:
        string vlanTagType @enum("c","s") = default("c");
        string nextProtocol = default("ethernetmac");
        int defaultVlanId = default(-1);
        int defaultPcp = default(-1);
        int defaultUserPriority = default(-1);
        int defaultDropEligible = default(-1); // <0: unspecified, 0: false, >0: true
        @class(Ieee8021qTagTpidHeaderInserter);
        @display("i=block/inserter");
}

