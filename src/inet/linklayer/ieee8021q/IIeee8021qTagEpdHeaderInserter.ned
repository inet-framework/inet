//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolHeaderInserter;

//
// Module interface for IEEE 802.1Q tag header inserters that add EPD (EtherType
// Protocol Discrimination) format headers to packets. Inserts VLAN tags with PCP
// (Priority Code Point), DEI (Drop Eligible Indicator), and VLAN ID fields.
// Supports both C-VLAN (0x8100) and S-VLAN (0x88A8) tag types through the
// vlanTagType parameter. Can use default values or extract values from packet
// tags. Used in VLAN-aware network devices to implement network segmentation and
// quality of service in Ethernet networks.
//
moduleinterface IIeee8021qTagEpdHeaderInserter extends IProtocolHeaderInserter
{
    parameters:
        string vlanTagType @enum("c","s");
        @display("i=block/inserter");
}
