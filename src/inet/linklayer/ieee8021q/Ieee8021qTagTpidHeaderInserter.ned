//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFlowBase;

//
// Concrete implementation of IEEE 802.1Q VLAN tag header inserter that adds VLAN tags to
// outgoing Ethernet frames using the TPID (Tag Protocol Identifier) field. Creates and
// inserts tag headers with the appropriate TPID value (0x8100 for C-TAG or 0x88A8 for
// S-TAG) along with Priority Code Point (PCP), VLAN Identifier (VID), and Drop Eligible
// Indicator (DEI) information.
//
// Tag field values are extracted from packet request tags (UserPriorityReq, PcpReq, VlanReq,
// DropEligibleReq) or use configured default values. The module can be configured to forward
// packets to a specific next protocol after tag insertion.
//
// @see IIeee8021qTagTpidHeaderInserter, OmittedIeee8021qTagTpidHeaderInserter
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
