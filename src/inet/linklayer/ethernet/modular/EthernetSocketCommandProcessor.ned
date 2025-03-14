//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Processes socket commands from applications and manages the Ethernet socket table.
// Handles bind, close, and destroy commands to create, modify, and remove socket
// entries in the socket table. This module enables applications to communicate
// directly with the Ethernet layer without requiring higher layer protocols.
//
// Socket commands are received as control information attached to Request messages.
// The module interprets these commands and updates the socket table accordingly:
// - EthernetBindCommand: Creates a new socket entry with specified parameters
// - SocketCloseCommand: Removes a socket entry and notifies the application
// - SocketDestroyCommand: Removes a socket entry without notification
//
// The module works in conjunction with EthernetSocketPacketProcessor, which
// handles the actual packet delivery to and from applications based on the
// socket information maintained in the socket table.
//
// @see ~EthernetSocketTable, ~EthernetSocketPacketProcessor, ~EthernetLayer
//
simple EthernetSocketCommandProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(EthernetSocketCommandProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}
