//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Packet processor for IEEE 802.2 LLC sockets that delivers incoming packets to registered applications. 
// Examines each packet's Service Access Point (SAP) identifiers to determine which sockets should receive 
// a copy. Forwards packets to applications that have registered interest in specific DSAP/SSAP combinations, 
// enabling protocol multiplexing at the LLC layer while maintaining normal packet flow.
//
// @see ~Ieee8022LlcSocketTable, ~Ieee8022LlcSocketCommandProcessor, ~IPacketFlow
//
simple Ieee8022LlcSocketPacketProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(Ieee8022LlcSocketPacketProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}
