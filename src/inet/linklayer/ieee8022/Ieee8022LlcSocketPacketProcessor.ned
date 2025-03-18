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
// Processes packets for IEEE 802.2 LLC protocol sockets. Examines incoming
// packets for their SAP (Service Access Point) values and delivers them to
// registered sockets. Duplicates packets for each matching socket based on
// source and destination SAP values, adding appropriate socket identification
// tags before forwarding to the upper layer. Works in conjunction with the
// socket table to provide socket-based access to LLC services.
//
simple Ieee8022LlcSocketPacketProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(Ieee8022LlcSocketPacketProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}
