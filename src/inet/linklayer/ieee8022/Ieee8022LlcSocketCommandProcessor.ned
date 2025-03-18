//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Processes socket commands for the IEEE 802.2 LLC protocol. Handles socket
// operations such as opening, closing, and destroying sockets. Manages the
// registration of sockets in the socket table, maintaining the mapping between
// socket IDs and LLC Service Access Points (SAPs). Works in conjunction with the
// Ieee8022LlcSocketTable to provide socket-based access to LLC services.
//
simple Ieee8022LlcSocketCommandProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(Ieee8022LlcSocketCommandProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}
