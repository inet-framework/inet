//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4layer.ipv4_modular;

import inet.queueing.base.PacketPusherBase;

//
// This module is responsible for routing packets that are to be sent locally.
// It does this by checking the destination address of the packet, determining
// the next hop address and the outgoing interface for the packet by consulting
// the routing table and the interface table, and adding that info to the packet
// in the form of tags. Different procedures are used for unicast, multicast, local, 
// limited broadcast, local broadcast, and other types of addresses. Pre-existing
// next hop address and outgoing interface request tags on the packet are honored.
//
simple Ipv4LocalOut extends PacketPusherBase
{
    parameters:
        @class(Ipv4LocalOut);
        string icmpModule; // ModuleReference
        string interfaceTableModule; // ModuleReference
        string routingTableModule; // ModuleReference
        //        string crcMode @enum("declared", "computed") = default("declared");
        //        int timeToLive = default(32);
        //        int multicastTimeToLive = default(32);
        bool limitedBroadcast = default(false); // send out limited broadcast packets comming from higher layer
        @signal[packetDropped](type=cPacket);
        @display("i=block/downarrow");
}
