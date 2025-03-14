//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketFlow;

//
// Forwarding module for Ethernet switches that determines the outgoing interface for each packet. 
// Consults the MAC forwarding table to find the appropriate interface based on the destination MAC 
// address. Handles unicast, multicast, and broadcast traffic with different forwarding strategies. 
// For unicast addresses, forwards to the specific interface; for unknown addresses or broadcasts, 
// duplicates the packet to all interfaces except the incoming one.
//
// @see ~IMacForwardingTable, ~MacForwardingTable, ~IInterfaceTable, ~RelayInterfaceLearner
//
simple RelayInterfaceSelector extends PacketPusherBase like IPacketFlow
{
    parameters:
        string macTableModule; // Relative module path of MAC table
        string interfaceTableModule; // Relative module path of interface table
        @class(RelayInterfaceSelector);
        @display("i=block/fork");
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropped](title="packets dropped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}
