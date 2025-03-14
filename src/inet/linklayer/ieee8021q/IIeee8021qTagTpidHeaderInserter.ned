//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolHeaderInserter;

//
// Module interface for IEEE 802.1Q VLAN tag header inserters that add VLAN tags to
// outgoing Ethernet frames using the TPID (Tag Protocol Identifier) field. These modules
// create and insert tag headers with the appropriate TPID value (0x8100 for C-TAG or
// 0x88A8 for S-TAG) along with Priority Code Point (PCP), VLAN Identifier (VID), and
// Drop Eligible Indicator (DEI) information.
//
// Implementations typically extract tag field values from packet request tags or use
// configured default values, enabling VLAN-based traffic segregation and Quality of Service.
//
// @see Ieee8021qTagTpidHeaderInserter, OmittedIeee8021qTagTpidHeaderInserter
//
moduleinterface IIeee8021qTagTpidHeaderInserter extends IProtocolHeaderInserter
{
    parameters:
        string vlanTagType @enum("c","s");
        @display("i=block/inserter");
}
