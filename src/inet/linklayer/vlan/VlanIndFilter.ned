//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// This module filters out packets based on the attached VlanInd tag.
//
simple VlanIndFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string interfaceTableModule;
        object acceptedVlanIds = default({}); // maps network interface name to a list of accepted VLAN ids, '*' as interface name matches all interfaces, -1 VLAN id matches if no tag is present
        @class(VlanIndFilter);
        @display("i=block/filter");
}

