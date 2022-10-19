//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

import inet.queueing.base.PacketPusherBase;


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
