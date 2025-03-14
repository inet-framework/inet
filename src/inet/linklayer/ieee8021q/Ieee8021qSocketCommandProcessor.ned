//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Command processor for IEEE 802.1Q sockets that handles socket operations like bind, close, and destroy. 
// Processes socket commands from upper layers and updates the socket table accordingly. Enables applications 
// to communicate with the IEEE 802.1Q protocol layer through a socket-based interface, similar to how 
// applications interact with transport layer protocols.
//
// @see ~Ieee8021qSocketTable, ~Ieee8021qSocketPacketProcessor, ~IPacketFlow
//
simple Ieee8021qSocketCommandProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(Ieee8021qSocketCommandProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}
