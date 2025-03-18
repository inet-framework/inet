//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolHeaderChecker;

//
// Module interface for IEEE 802.1Q tag header checkers that process EPD
// (EtherType Protocol Discrimination) format headers. Validates and extracts
// information from VLAN tags including VLAN ID, PCP (Priority Code Point), and
// DEI (Drop Eligible Indicator). Supports both C-VLAN (0x8100) and S-VLAN
// (0x88A8) tag types through the vlanTagType parameter. Can filter packets based
// on VLAN IDs and determines the next protocol based on the typeOrLength field.
// Used in VLAN-aware network devices implementing the IEEE 802.1Q standard.
//
moduleinterface IIeee8021qTagEpdHeaderChecker extends IProtocolHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s");
        @display("i=block/checker");
}
