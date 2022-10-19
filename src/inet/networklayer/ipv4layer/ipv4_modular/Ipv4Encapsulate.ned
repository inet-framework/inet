//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

import inet.queueing.base.PacketPusherBase;


simple Ipv4Encapsulate extends PacketPusherBase
{
    parameters:
        @class(Ipv4Encapsulate);
        string interfaceTableModule; // ModuleReference
        string routingTableModule; // ModuleReference
        string crcMode @enum("declared","computed") = default("declared");
        int timeToLive = default(32);
        int multicastTimeToLive = default(32);
        @signal[packetReceivedFromUpper](type=cPacket);
        @display("i=block/inserter");
}
