//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8021q;

import inet.protocolelement.contract.IProtocolHeaderInserter;

//
// Module interface for IEEE 802.1Q tag header inserters that add TPID (Tag
// Protocol Identifier) fields to Ethernet frames. Supports both C-VLAN (0x8100)
// and S-VLAN (0x88A8) tag types through the vlanTagType parameter. These
// headers contain VLAN ID, priority (PCP), and drop eligibility (DEI) fields
// that enable network segmentation, quality of service, and congestion
// management in Ethernet networks. Used in the implementation of Virtual LAN
// functionality according to the IEEE 802.1Q standard.
//
moduleinterface IIeee8021qTagTpidHeaderInserter extends IProtocolHeaderInserter
{
    parameters:
        string vlanTagType @enum("c","s");
        @display("i=block/inserter");
}
