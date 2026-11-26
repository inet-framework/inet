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
// A map to -1 sends the frame untagged on that interface (the untagged set of
// IEEE 802.1Q-2018 8.8.2): the module removes the VLAN, PCP and drop eligible
// requests of its tag type and requests no encapsulation of its tag protocol.
// The priority of the frame stays as its user priority (`UserPriorityReq`).
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

