//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolHeaderChecker;

//
// Module interface for IEEE 802.1Q VLAN tag header checkers that process Ethernet frames
// with VLAN tags using the TPID (Tag Protocol Identifier) field. These modules verify
// the TPID field to identify VLAN tags and extract tag information including Priority
// Code Point (PCP), VLAN Identifier (VID), and Drop Eligible Indicator (DEI).
//
// Implementations can support different VLAN tag types (C-TAG with TPID 0x8100 or
// S-TAG with TPID 0x88A8) and may provide filtering capabilities based on VLAN IDs.
//
// @see Ieee8021qTagTpidHeaderChecker, OmittedIeee8021qTagTpidHeaderChecker
//
moduleinterface IIeee8021qTagTpidHeaderChecker extends IProtocolHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s");
        @display("i=block/checker");
}
