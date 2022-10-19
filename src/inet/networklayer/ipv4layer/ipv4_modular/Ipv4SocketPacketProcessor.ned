//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.ipv4layer.ipv4_modular;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketFlow;

simple Ipv4SocketPacketProcessor extends PacketPusherBase like IPacketFlow
{
    parameters:
        @class(Ipv4SocketPacketProcessor);
        @display("i=block/cogwheel");
        string icmpModule;
        string socketTableModule;
        @signal[packetSentToUpper](type=cPacket);
    gates:
        input socketIn;
}
