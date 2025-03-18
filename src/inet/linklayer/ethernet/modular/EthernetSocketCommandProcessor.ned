//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Processes socket commands for the Ethernet protocol. Handles socket operations
// such as opening, closing, and destroying sockets. Manages the registration of
// sockets in the socket table, maintaining the mapping between socket IDs and
// Ethernet parameters. Works in conjunction with the EthernetSocketTable to
// provide socket-based access to Ethernet services.
//
simple EthernetSocketCommandProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(EthernetSocketCommandProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}
