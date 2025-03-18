//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFilterBase;

//
// Validates and processes IEEE 802.1Q tag headers based on the Tag Protocol
// Identifier (TPID) field. Verifies that the TPID matches the expected value
// (0x8100 for C-VLAN or 0x88A8 for S-VLAN) and extracts VLAN ID, PCP (Priority
// Code Point), and DEI (Drop Eligible Indicator) information. Adds corresponding
// indication tags to packets and can filter based on VLAN IDs using the
// vlanIdFilter parameter. Used in VLAN-aware network devices to implement
// traffic filtering and Quality of Service (QoS) mechanisms.
//
simple Ieee8021qTagTpidHeaderChecker extends PacketFilterBase like IIeee8021qTagTpidHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s") = default("c");
        object vlanIdFilter = default([]);
        @class(Ieee8021qTagTpidHeaderChecker);
        @display("i=block/checker");
}

