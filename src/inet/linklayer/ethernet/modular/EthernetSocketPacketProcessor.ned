//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketFlow;

//
// Processes incoming Ethernet packets and delivers them to the appropriate
// applications based on socket registrations. Examines each packet's MAC addresses
// and protocol, then looks up matching sockets in the socket table to determine
// which applications should receive the packet.
//
// For each matching socket found in the socket table, the module creates a copy
// of the packet, tags it with the socket ID, and forwards it to the application.
// The original packet is either:
// - Deleted if any matching socket has the "steal" flag set (indicating exclusive
//   delivery to the application)
// - Forwarded to the next module in the processing chain if no socket steals it
//
// Works in conjunction with EthernetSocketCommandProcessor and EthernetSocketTable
// to provide a complete socket-based interface for applications to directly
// communicate using Ethernet frames without higher layer protocols.
//
// @see ~EthernetSocketTable, ~EthernetSocketCommandProcessor, ~EthernetLayer
//
simple EthernetSocketPacketProcessor extends PacketPusherBase like IPacketFlow
{
    parameters:
        @class(EthernetSocketPacketProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}
