//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

simple Ipv4LocalOut
{
    parameters:
        string icmpModule; // ModuleReference
        string interfaceTableModule; // ModuleReference
        string routingTableModule; // ModuleReference
//        string crcMode @enum("declared", "computed") = default("declared");
//        int timeToLive = default(32);
//        int multicastTimeToLive = default(32);
        bool limitedBroadcast = default(false); // send out limited broadcast packets comming from higher layer
        string displayStringTextFormat = default("");   // TODO required for PacketProcessorBase
        @signal[packetDropped](type=cPacket);
    gates:
        input in;
        output out;
}
