//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.common;

import inet.networklayer.configurator.contract.INetworkConfigurator;

//
// Provides automatic configuration for Time-Sensitive Networking (TSN) stream
// redundancy mechanisms, enabling critical data streams to be transmitted over
// multiple paths simultaneously for enhanced reliability. This module configures
// the necessary components for stream splitting at source nodes, stream merging
// at destination nodes, and appropriate filtering at intermediate nodes.
//
// Stream redundancy is a key TSN feature that:
// - Protects against link and node failures by using multiple paths
// - Reduces end-to-end latency variation by selecting the first arriving copy
// - Improves overall reliability for critical traffic
//
// The configurator automatically:
// - Identifies source, destination, and intermediate nodes for each stream
// - Configures stream splitters at source nodes to duplicate packets
// - Sets up stream mergers at destination nodes to eliminate duplicates
// - Assigns appropriate VLAN IDs to distinguish redundant streams
// - Establishes filtering rules at intermediate nodes
//
// The configuration uses a structured object format where each stream specifies:
// - Stream identification (name, packet filter)
// - Source and destination nodes
// - Multiple redundant paths through the network
//
// Each path is defined as a sequence of nodes that the stream should traverse,
// allowing precise control over how redundant copies are routed through the network.
//
// @see ~FailureProtectionConfigurator
//
simple StreamRedundancyConfigurator like INetworkConfigurator
{
    parameters:
        int minVlanId = default(0); // Lowest available VLAN ID
        int maxVlanId = default(4095); // Highest available VLAN ID
        object configuration @mutable = default([]); // A vector of objects (e.g. [{...}, {...}]) where each object has the following fields: name, source, destination, packetFilter, paths
                                                     // Here is an example: [{name: "S1", packetFilter: "*", source: "source", destination: "destination", trees: [[["s1", "s2a", "s3a"]], [["s1", "s2b", "s3b"]], [["s1", "s2a", "s2b", "s3b"]], [["s1", "s2b", "s2a", "s3a"]]]}]
        @display("i=block/cogwheel");
}
