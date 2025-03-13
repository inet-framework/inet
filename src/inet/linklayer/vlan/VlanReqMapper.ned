//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Transforms VLAN IDs on outgoing packets by updating the `VlanReq` tag according
// to a configurable mapping. This module enables VLAN translation, which is essential
// for connecting different VLAN domains, implementing VLAN hierarchies, or enforcing
// network segmentation policies.
//
// The module uses the `mappedVlanIds` parameter to determine how to transform VLAN IDs.
// This parameter maps network interface names to another map which maps source VLAN IDs
// to destination VLAN IDs. Special values include:
// - '*' as interface name: Applies to all interfaces
// - -1 as VLAN ID: Represents packets without a VLAN tag
//
// Common use cases include:
// - VLAN translation at network boundaries
// - Implementing Q-in-Q tunneling (VLAN stacking)
// - Normalizing VLAN IDs across different network segments
//
// @see ~VlanReqFilter, ~VlanPolicyLayer
//
simple VlanReqMapper extends PacketFlowBase like IPacketFlow
{
    parameters:
        string interfaceTableModule;
        string protocol = default("ieee8021qctag");
        object mappedVlanIds = default({}); // Maps network interface name to another map which maps accepted VLAN ids to replacement VLAN ids, '*' as interface name matches all interfaces, -1 VLAN id matches if no tag is present
        @class(VlanReqMapper);
        @display("i=block/switch");
}
