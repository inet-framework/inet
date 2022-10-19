//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

import inet.queueing.base.PacketPusherBase;


simple Ipv4Fragmenter extends PacketPusherBase
{
    parameters:
        @class(Ipv4Fragmenter);
        string arpModule;
        string icmpModule;
        string interfaceTableModule;
        string crcMode @enum("declared","computed");
        int timeToLive;
        int multicastTimeToLive;
        @signal[packetDropped](type=cPacket);
        @display("i=block/frag");
}
