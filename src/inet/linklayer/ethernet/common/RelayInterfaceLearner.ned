//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// This module extracts the source MAC address of the packet passing through and
// stores the mapping from this MAC address to the incoming network interface in
// the MAC address table (forwarding information database).
//
simple RelayInterfaceLearner extends PacketFlowBase like IPacketFlow
{
    parameters:
        string macTableModule; // relative module path of MAC table
        string interfaceTableModule; // relative module path of interface table
        @class(RelayInterfaceLearner);
        @display("i=block/blackboard");
}

