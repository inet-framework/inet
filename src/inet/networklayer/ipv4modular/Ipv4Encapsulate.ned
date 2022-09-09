//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

simple Ipv4Encapsulate
{
    parameters:
        string interfaceTableModule; // ModuleReference
        string routingTableModule; // ModuleReference
        string crcMode @enum("declared","computed") = default("declared");
        int timeToLive = default(32);
        int multicastTimeToLive = default(32);
        string displayStringTextFormat = default("");   // TODO required for PacketProcessorBase
        @signal[packetReceivedFromUpper](type=cPacket);
        @display("i=block/inserter");
    gates:
        input in;
        output out;
}
