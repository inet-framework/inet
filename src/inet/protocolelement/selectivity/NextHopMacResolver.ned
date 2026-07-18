//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.queueing.base.PacketFlowBase;

//
// Resolves the next-hop L3 address (a NextHopAddressReq tag set by ~Forwarding) to a link-layer
// MacAddressReq using a static, configured neighbour table -- a minimal static "ARP". On a shared
// medium this is what lets ~SendToMacAddress unicast a forwarded packet to the intended next hop.
//
simple NextHopMacResolver extends PacketFlowBase
{
    parameters:
        @class(NextHopMacResolver);
        string neighbors = default(""); // static table: "l3address macaddress; l3address macaddress; ..."
        @display("i=block/layer");
}
