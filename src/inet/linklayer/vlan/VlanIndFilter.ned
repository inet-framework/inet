//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Filters incoming packets based on their VLAN ID as indicated by the attached
// `VlanInd` tag. This module allows selective processing of packets from specific
// VLANs, enabling VLAN-based traffic segregation and policy enforcement.
//
// The module uses the `acceptedVlanIds` parameter to determine which packets to
// accept or drop. This parameter maps network interface names to lists of accepted
// VLAN IDs. Special values include:
// - '*' as interface name: Matches all interfaces
// - -1 as VLAN ID: Matches packets without a VLAN tag
//
// Packets that don't match the acceptance criteria are dropped, while matching
// packets are forwarded to the output gate for further processing.
//
// @see ~VlanReqFilter, ~VlanPolicyLayer
//
simple VlanIndFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string interfaceTableModule;
        object acceptedVlanIds = default({}); // Maps network interface name to a list of accepted VLAN ids, '*' as interface name matches all interfaces, -1 VLAN id matches if no tag is present
        @class(VlanIndFilter);
        @display("i=block/filter");
}
