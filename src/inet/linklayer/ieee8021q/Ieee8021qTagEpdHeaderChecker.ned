//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFilterBase;

//
// Concrete implementation of IEEE 802.1Q VLAN tag header checker that processes Ethernet
// frames with VLAN tags using the EPD (EtherType Protocol Discrimination) method. Extracts
// VLAN tag information including Priority Code Point (PCP), VLAN Identifier (VID), and
// Drop Eligible Indicator (DEI), and adds corresponding indication tags to the packet.
//
// Supports filtering packets based on VLAN IDs through the vlanIdFilter parameter, and
// can be configured to handle either C-TAG (customer, 0x8100) or S-TAG (service, 0x88A8)
// VLAN tags.
//
// @see IIeee8021qTagEpdHeaderChecker, OmittedIeee8021qTagEpdHeaderChecker
//
simple Ieee8021qTagEpdHeaderChecker extends PacketFilterBase like IIeee8021qTagEpdHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s") = default("c");
        object vlanIdFilter @mutable = default([]);
        @class(Ieee8021qTagEpdHeaderChecker);
        @display("i=block/checker");
}
