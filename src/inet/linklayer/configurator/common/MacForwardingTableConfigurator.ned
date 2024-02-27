//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator;

import inet.networklayer.configurator.contract.INetworkConfigurator;

//
// This module configures the forwarding database (MAC address table) of all
// network nodes in the network based on the automatically discovered network
// topology. The configurator uses the shortest path algorithm to determine the
// outgoing network interface for all network nodes and all destination network
// interfaces. The effect of this configuration is that the network can use the
// ~GlobalArp module and completely eliminate the ARP protocol messages.
//
// @see ~MacForwardingTable
//
simple MacForwardingTableConfigurator like INetworkConfigurator
{
    parameters:
        @display("i=block/cogwheel");
}

