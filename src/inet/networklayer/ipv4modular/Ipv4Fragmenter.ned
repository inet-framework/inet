//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

simple Ipv4Fragmenter
{
    parameters:
        string arpModule;
        string icmpModule;
        string interfaceTableModule;
        string crcMode @enum("declared","computed");
        int timeToLive;
        int multicastTimeToLive;
        string displayStringTextFormat = default("");   // TODO required for PacketProcessorBase
        @signal[packetDropped](type=cPacket);
        @display("i=block/frag");
    gates:
        input in;
        output out;
}
