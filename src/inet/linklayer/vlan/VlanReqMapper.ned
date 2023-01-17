//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// This module updates the VlanReq tag on packets.
//
simple VlanReqMapper extends PacketFlowBase like IPacketFlow
{
    parameters:
        string interfaceTableModule;
        string protocol = default("ieee8021qctag");
        object mappedVlanIds = default({}); // maps network interface name to another map which maps accepted VLAN ids to replacement VLAN ids, '*' as interface name matches all interfaces, -1 VLAN id matches if no tag is present
        @class(VlanReqMapper);
        @display("i=block/switch");
}

