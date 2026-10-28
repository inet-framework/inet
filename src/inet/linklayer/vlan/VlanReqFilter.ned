//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Filters out packets based on the attached VLAN request tag (`CVlanReq` or
// `SVlanReq`, see `vlanTagType`).
//
simple VlanReqFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string interfaceTableModule;
        string vlanTagType @enum("c","s") = default("c"); // The VLAN tag whose tags the module uses: "c" for the C-tag (CVlanReq, CVlanInd), "s" for the S-tag (SVlanReq, SVlanInd)
        object acceptedVlanIds = default({}); // Maps network interface name to a list of accepted VLAN ids, '*' as interface name matches all interfaces, -1 VLAN id matches if no tag is present
        @class(VlanReqFilter);
        @display("i=block/filter");
}

