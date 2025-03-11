//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.flow;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Initiates detailed packet flow measurements by marking packets and starting
// the tracking of various timing metrics. Works in conjunction with the
// FlowMeasurementRecorder to provide comprehensive packet flow analysis.
//
// Flow measurement capabilities:
// - Marks packets to begin bit-level tracking through the network
// - Configurable to measure specific packet types or parts of packets
// - Initiates timing measurements for queueing, processing, transmission, etc.
// - Creates a unique flow identity that can be recognized by recorders
//
// Key features:
// - Bit-level granularity for precise measurements
// - Flexible packet filtering
// - Customizable measurement regions within packets
// - Named flows to separate different measurement scenarios
// - Selective measurement of specific timing components
//
// Applications:
// - Performance analysis of network protocols
// - End-to-end delay measurement
// - Protocol optimization
// - Quality of Service (QoS) validation
// - Network bottleneck identification
// - Comparative analysis of different network configurations
//
// Typical usage involves placing a FlowMeasurementStarter at the beginning
// of a network path and one or more FlowMeasurementRecorder modules at points
// of interest along the path. The starter initiates the measurement process,
// while the recorders collect and analyze the timing data.
//
// The starter can be configured to measure only specific parts of packets
// (using offset and length parameters) and to track only certain timing
// components (using the measure parameter).
//
// @see ~FlowMeasurementRecorder
//
simple FlowMeasurementStarter extends PacketFlowBase like IPacketFlow
{
    parameters:
        object packetFilter = default("*"); // Specifies which packets are considered, matches all packets by default
        int offset @unit(b) = default(0b); // Designates the packet part to be measured
        int length @unit(b) = default(-1b); // Designates the packet part to be measured
        string flowName = default(""); // Use packet flow to separate measurement from other measurements
        string measure = default("not packetEvent"); // Match expression for a combination of: packetEvent, elapsedTime, delayingTime, queueingTime, processingTime, transmissionTime, propagationTime
        @class(FlowMeasurementStarter);
        @display("i=block/timer");
        @signal[packetFlowStarted](type=inet::Packet);
}
