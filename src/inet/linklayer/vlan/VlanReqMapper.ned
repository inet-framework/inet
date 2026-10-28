//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Updates the VLAN request tag (`CVlanReq` or `SVlanReq`, see `vlanTagType`)
// on packets.
//
simple VlanReqMapper extends PacketFlowBase like IPacketFlow
{
    parameters:
        string interfaceTableModule;
        string vlanTagType @enum("c","s") = default("c"); // The VLAN tag whose tags the module uses, and whose protocol it requests: "c" for the C-tag (CVlanReq, CVlanInd), "s" for the S-tag (SVlanReq, SVlanInd)
        object mappedVlanIds = default({}); // Maps network interface name to another map which maps accepted VLAN ids to replacement VLAN ids, '*' as interface name matches all interfaces, -1 VLAN id matches if no tag is present
        @class(VlanReqMapper);
        @display("i=block/switch");
}

