//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Filters outgoing packets based on their VLAN ID as specified by the attached
// `VlanReq` tag. This module enables VLAN-based traffic control by allowing only
// packets with approved VLAN IDs to be transmitted on specific interfaces.
//
// The module uses the `acceptedVlanIds` parameter to determine which packets to
// accept or drop. This parameter maps network interface names to lists of accepted
// VLAN IDs. Special values include:
// - '*' as interface name: Matches all interfaces
// - -1 as VLAN ID: Matches packets without a VLAN tag
//
// This filter is typically used in the outbound path of a VLAN policy layer to
// enforce VLAN transmission policies before packets reach the MAC layer.
//
// @see ~VlanIndFilter, ~VlanPolicyLayer, ~VlanReqMapper
//
simple VlanReqFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string interfaceTableModule;
        object acceptedVlanIds = default({}); // Maps network interface name to a list of accepted VLAN ids, '*' as interface name matches all interfaces, -1 VLAN id matches if no tag is present
        @class(VlanReqFilter);
        @display("i=block/filter");
}
