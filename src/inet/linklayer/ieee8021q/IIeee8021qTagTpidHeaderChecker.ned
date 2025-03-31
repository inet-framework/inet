//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolHeaderChecker;

//
// Module interface for IEEE 802.1Q VLAN tag header checkers that verify and
// process Tag Protocol Identifier (TPID) fields in Ethernet frames. Supports both
// Customer VLAN (C-VLAN, TPID 0x8100) and Service VLAN (S-VLAN, TPID 0x88A8)
// tag types. Filters packets based on VLAN IDs, extracts VLAN tag information
// (PCP, DEI, VID), and adds it as packet tags. Used in VLAN-aware network
// devices to implement traffic filtering and Quality of Service (QoS) through
// Priority Code Point (PCP) handling.
//
moduleinterface IIeee8021qTagTpidHeaderChecker extends IProtocolHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s");
        @display("i=block/checker");
}
