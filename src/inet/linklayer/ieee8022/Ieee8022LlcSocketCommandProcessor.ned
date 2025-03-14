//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Command processor for IEEE 802.2 LLC sockets that handles socket operations like open, close, and destroy. 
// Processes socket commands from upper layers and updates the socket table accordingly. Enables applications 
// to communicate with the LLC protocol layer through a socket-based interface, allowing them to send and 
// receive frames with specific Service Access Point (SAP) identifiers.
//
// @see ~Ieee8022LlcSocketTable, ~Ieee8022LlcSocketPacketProcessor, ~IPacketFlow
//
simple Ieee8022LlcSocketCommandProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(Ieee8022LlcSocketCommandProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}
