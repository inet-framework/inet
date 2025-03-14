//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFlowBase;

//
// Concrete implementation of IEEE 802.1Q VLAN tag header inserter that adds VLAN tags to
// outgoing Ethernet frames using the EPD (EtherType Protocol Discrimination) method. Creates
// and inserts tag headers containing Priority Code Point (PCP), VLAN Identifier (VID), and
// Drop Eligible Indicator (DEI) information.
//
// Tag field values are extracted from packet request tags (UserPriorityReq, PcpReq, VlanReq,
// DropEligibleReq) or use configured default values. Supports both C-TAG (customer, 0x8100)
// and S-TAG (service, 0x88A8) VLAN tag types.
//
// @see IIeee8021qTagEpdHeaderInserter, OmittedIeee8021qTagEpdHeaderInserter
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
