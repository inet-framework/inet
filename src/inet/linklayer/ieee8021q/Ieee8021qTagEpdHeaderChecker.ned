//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFilterBase;

//
// Validates and processes IEEE 802.1Q tag headers in EPD (EtherType Protocol
// Discrimination) format. Extracts VLAN ID, PCP (Priority Code Point), and DEI
// (Drop Eligible Indicator) information from incoming packets and adds
// corresponding indication tags. Supports both C-VLAN (0x8100) and S-VLAN
// (0x88A8) tag types. Can filter packets based on VLAN IDs using the
// vlanIdFilter parameter. Determines the next protocol based on the typeOrLength
// field and registers appropriate protocol services for IEEE 802.1Q processing.
//
simple Ieee8021qTagEpdHeaderChecker extends PacketFilterBase like IIeee8021qTagEpdHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s") = default("c");
        object vlanIdFilter @mutable = default([]);
        @class(Ieee8021qTagEpdHeaderChecker);
        @display("i=block/checker");
}

