//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.common;

import inet.networklayer.configurator.contract.INetworkConfigurator;

//
// Provides comprehensive failure protection for Time-Sensitive Networking (TSN)
// by coordinating multiple configurator modules to ensure reliable data delivery
// even when network components fail. This module acts as a high-level orchestrator
// that combines stream redundancy and gate scheduling configurations.
//
// The configurator creates resilient network paths by:
// - Coordinating with the stream redundancy configurator to establish multiple
//   redundant paths for critical data streams
// - Working with the gate schedule configurator to ensure proper timing and
//   bandwidth allocation for all paths
// - Analyzing potential failure scenarios and creating appropriate protection mechanisms
//
// Key protection capabilities:
// - Link failure protection: Ensures streams can survive the failure of specific links
//   by routing traffic through alternative paths
// - Node failure protection: Maintains connectivity even when network nodes fail
//   by establishing disjoint paths through the network
// - Configurable redundancy levels: Allows specifying exactly how many simultaneous
//   failures the system should tolerate
//
// The configuration uses a structured object format where each stream can specify:
// - Basic properties (name, source, destination, packet characteristics)
// - Quality of service requirements (maximum latency, priority)
// - Failure protection requirements (which links/nodes might fail and how many
//   simultaneous failures to protect against)
// - Maximum redundancy level (limiting resource usage for protection)
//
// @see ~StreamRedundancyConfigurator
//
simple FailureProtectionConfigurator like INetworkConfigurator
{
    parameters:
        string streamRedundancyConfiguratorModule = default("streamRedundancyConfigurator");
        string gateScheduleConfiguratorModule = default("gateScheduleConfigurator");
        object configuration = default([]); // A vector of objects (e.g. [{...}, {...}]) where each object has the following fields: name, type, application, source, destination, PCP, gate index, packetFilter, packetLength, packetInterval, maxLatency, linkFailureProtection, nodeFailureProtection, maxRedundancy
                                            // here is an example: [{name: "s1", application: "app[0]", source: "source", destination: "destination", pcp: 0, gateIndex: 0, packetFilter: "*", packetLength: 200B, packetInterval: 500us, maxLatency: 100us, linkFailureProtection: [{any: 1, of: "*"}], maxRedundancy: 2}]
        @display("i=block/cogwheel");
}
