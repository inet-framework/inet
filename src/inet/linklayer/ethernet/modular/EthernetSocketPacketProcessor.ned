//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketFlow;

//
// Processes packets for Ethernet sockets. Handles incoming packets by delivering
// them to the appropriate socket based on packet matching criteria. Manages the
// packet flow between the network interface and the socket layer. Works in
// conjunction with the EthernetSocketTable and EthernetSocketCommandProcessor to
// provide socket-based access to Ethernet services.
//
simple EthernetSocketPacketProcessor extends PacketPusherBase like IPacketFlow
{
    parameters:
        @class(EthernetSocketPacketProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}

