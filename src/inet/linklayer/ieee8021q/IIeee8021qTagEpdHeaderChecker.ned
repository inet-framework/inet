//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolHeaderChecker;

//
// Module interface for IEEE 802.1Q VLAN tag header checkers that process Ethernet frames
// with VLAN tags. These modules extract VLAN tag information from incoming packets,
// including Priority Code Point (PCP), VLAN Identifier (VID), and Drop Eligible Indicator (DEI).
// They also determine the encapsulated protocol based on the EtherType/Length field.
//
// Implementations can support different VLAN tag types (C-TAG or S-TAG) and may provide
// filtering capabilities based on VLAN IDs.
//
// @see Ieee8021qTagEpdHeaderChecker, OmittedIeee8021qTagEpdHeaderChecker
//
moduleinterface IIeee8021qTagEpdHeaderChecker extends IProtocolHeaderChecker
{
    parameters:
        string vlanTagType @enum("c","s");
        @display("i=block/checker");
}
