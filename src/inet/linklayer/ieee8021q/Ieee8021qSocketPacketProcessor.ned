//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketFlow;

//
// Packet processor for IEEE 802.1Q sockets that delivers incoming packets to registered applications. 
// Examines each packet's protocol and VLAN ID to determine which sockets should receive a copy. 
// Supports packet stealing where a socket can prevent the packet from being delivered to the normal 
// processing path, enabling applications to intercept specific VLAN traffic.
//
// @see ~Ieee8021qSocketTable, ~Ieee8021qSocketCommandProcessor, ~IPacketFlow
//
simple Ieee8021qSocketPacketProcessor extends PacketPusherBase like IPacketFlow
{
    parameters:
        @class(Ieee8021qSocketPacketProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}
