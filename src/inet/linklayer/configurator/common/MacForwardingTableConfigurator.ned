//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.common;

import inet.networklayer.configurator.contract.INetworkConfigurator;

//
// Provides automatic configuration of MAC forwarding tables (also known as
// forwarding databases or FDBs) for all network nodes based on the discovered
// network topology. This configurator eliminates the need for dynamic MAC
// address learning in switched networks.
//
// The configurator operates by:
// - Discovering the complete network topology
// - Computing optimal paths between all node pairs using shortest path algorithms
// - Populating MAC forwarding tables with pre-computed entries
// - Configuring each node with the appropriate forwarding information
//
// Key benefits:
// - Eliminates the learning phase in switched networks
// - Reduces network convergence time
// - Enables the use of ~GlobalArp to eliminate ARP protocol messages
// - Provides deterministic forwarding behavior
//
// For each combination of source node and destination MAC address, this module
// determines the optimal outgoing interface based on the network topology.
// This static configuration approach is particularly useful in simulation
// environments where dynamic learning would add unnecessary complexity.
//
// @see ~MacForwardingTable, ~GlobalArp
//
simple MacForwardingTableConfigurator like INetworkConfigurator
{
    parameters:
        @display("i=block/cogwheel");
}
