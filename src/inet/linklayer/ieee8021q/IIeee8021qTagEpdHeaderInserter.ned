//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolHeaderInserter;

//
// Module interface for IEEE 802.1Q VLAN tag header inserters that add VLAN tags to
// outgoing Ethernet frames. These modules create and insert tag headers containing
// Priority Code Point (PCP), VLAN Identifier (VID), and Drop Eligible Indicator (DEI)
// information, enabling VLAN-based traffic segregation and Quality of Service.
//
// Implementations can support different VLAN tag types (C-TAG or S-TAG) and typically
// extract tag field values from packet request tags or use configured default values.
//
// @see Ieee8021qTagEpdHeaderInserter, OmittedIeee8021qTagEpdHeaderInserter
//
moduleinterface IIeee8021qTagEpdHeaderInserter extends IProtocolHeaderInserter
{
    parameters:
        string vlanTagType @enum("c","s");
        @display("i=block/inserter");
}
