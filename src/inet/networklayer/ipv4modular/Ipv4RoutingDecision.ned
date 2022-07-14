//
// Copyright (C) 2022 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ipv4modular;

import inet.queueing.base.PacketPusherBase;


simple Ipv4RoutingDecision extends PacketPusherBase
{
    parameters:
        @class(Ipv4RoutingDecision);
        string icmpModule;
        string interfaceTableModule;
        string routingTableModule;
        @signal[packetDropped](type=cPacket);
        @signal[ipv4NewMulticast](type=inet::Ipv4Header);
        @signal[ipv4DataOnNonrpf](type=inet::Ipv4Header);
        @signal[ipv4DataOnRpf](type=inet::Ipv4Header);
        @signal[ipv4MdataRegister](type=inet::Packet);
        @display("i=block/network2");
}
