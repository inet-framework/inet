//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.gatescheduling.base;

//
// Base module for gate scheduling configurators that provides infrastructure for
// Time-Sensitive Networking (TSN) gate control. Extracts input parameters from network
// topology and configures the resulting schedule in PeriodicGate submodules across the
// network. Handles configuration of gate cycles, application start offsets, and packet
// flow specifications, allowing derived modules to focus on implementing specific
// scheduling algorithms.
//
// @see AlwaysOpenGateScheduleConfigurator, EagerGateScheduleConfigurator, TSNschedGateScheduleConfigurator, Z3GateScheduleConfigurator, IGateScheduleConfigurator
//
// The configuration is an array of objects where each object describes a packet
// flow through the network.
//   - name: the name of the flow
//   - PCP: the PCP header field of the packets
//   - `gateIndex`: the gate index of the traffic class in all network interfaces along the path
//   - application (optional): the name of the application module in the source where the start time will be configured, not used by default
//   - source: the network node name of the packet flow source
//   - destination: the network node name of the packet flow destination
//   - `packetLength`: the expected packet length of the packets in the flow (may also be used to have extra margin in the schedule)
//   - `packetInterval`: the expected interval between packets
//   - `maxLatency` (optional): the allowed maximum end-to-end delay of packets, not used by default
//   - `maxJitter` (optional): the allowed maximum jitter among the packets, 0 by default
//   - `pathFragments` (optional): array of arrays, where each array contains a list of network node names, supports multicast trees, shortest path is used when omitted
//
simple GateScheduleConfiguratorBase
{
    parameters:
        double gateCycleDuration @unit(s); // The globally used gate scheduling period in each PeriodicGate module
        object configuration @mutable = default([]); // Array of objects, see ~GateScheduleConfiguratorBase module documentation for more details
                                                     // example: [{name: "s1", type: "unicast", application: "app[0]", pcp: 0, gateIndex: 0, source: "wheel*", destination: "hud", packetLength: 100B, packetInterval: 5ms, maxLatency: 100us, maxJitter: 10us, pathFragments: [["a", "b", "c"]]}]
        @display("i=block/cogwheel");
}
