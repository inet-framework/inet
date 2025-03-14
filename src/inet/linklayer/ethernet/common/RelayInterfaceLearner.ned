//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Learning module for Ethernet switches that builds the MAC address forwarding table. Examines the source 
// MAC address of each passing packet and associates it with the incoming interface in the MAC forwarding 
// table. Implements the self-learning mechanism of Ethernet switches, enabling them to forward frames to 
// the correct port based on destination MAC addresses.
//
// @see ~IMacForwardingTable, ~MacForwardingTable, ~IInterfaceTable, ~PacketFlowBase
//
simple RelayInterfaceLearner extends PacketFlowBase like IPacketFlow
{
    parameters:
        string macTableModule; // Relative module path of MAC table
        string interfaceTableModule; // Relative module path of the interface table
        @class(RelayInterfaceLearner);
        @display("i=block/blackboard");
}
