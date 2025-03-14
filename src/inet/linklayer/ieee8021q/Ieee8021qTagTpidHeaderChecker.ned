//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFilterBase;

//
// Concrete implementation of IEEE 802.1Q VLAN tag header checker that processes Ethernet
// frames with VLAN tags using the TPID (Tag Protocol Identifier) field. Verifies the TPID
// field (0x8100 for C-TAG or 0x88A8 for S-TAG) to identify VLAN tags and extracts tag
// information including Priority Code Point (PCP), VLAN Identifier (VID), and Drop
// Eligible Indicator (DEI).
//
// Supports filtering packets based on VLAN IDs through the vlanIdFilter parameter, and
// can be configured to handle either C-TAG (customer) or S-TAG (service) VLAN tags.
//
// @see IIeee8021qTagTpidHeaderChecker, OmittedIeee8021qTagTpidHeaderChecker
//
simple Ieee8021qTagTpidHeaderChecker extends PacketFilterBase like IIeee8021qTagTpidHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s") = default("c");
        object vlanIdFilter = default([]);
        @class(Ieee8021qTagTpidHeaderChecker);
        @display("i=block/checker");
}
