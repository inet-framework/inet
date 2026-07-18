//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.forwarding;

import inet.queueing.base.PacketPusherBase;

//
// Hop-by-hop forwarding element. A packet carrying a DestinationL3AddressHeader is either
// delivered locally (when its destination equals this node's ~address) or forwarded towards
// the next hop given by the static ~routes table.
//
simple Forwarding extends PacketPusherBase
{
    parameters:
        @class(Forwarding);
        string address = default("");             // this node's L3 address; a packet destined here is delivered locally
        string routes = default("");              // static routing table: "destination nextHop [interface]" entries separated by ';'
        string interfaceTableModule = default(""); // when set, the interface field of each route is an index into this table (otherwise a literal interface id)
        @display("i=block/tx");
}
