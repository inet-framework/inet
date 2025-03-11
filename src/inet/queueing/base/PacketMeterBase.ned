//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet meter components in the queueing framework.
// Packet meters measure various characteristics of packet flows and
// attach tags with the measurement results for use by other modules.
//
// Packet meters are essential for implementing:
// - Traffic monitoring and measurement
// - Rate limiting and token bucket algorithms
// - Quality of Service (QoS) mechanisms
// - Bandwidth management
// - Traffic analysis and statistics collection
//
// Meters typically operate by:
// 1. Receiving packets (either pushed to their input or pulled from their output)
// 2. Measuring packet characteristics (size, rate, timing, etc.)
// 3. Attaching tags with measurement results
// 4. Forwarding packets without changing their order or timing
//
// Measurements can focus on various packet characteristics:
// - Packet sizes and bit rates
// - Inter-arrival times and jitter
// - Flow statistics and aggregated metrics
// - Protocol-specific properties
// - Conformance to traffic specifications
//
// Derived meter modules implement various metering strategies including:
// - Token bucket metering (for rate limiting)
// - Sliding window rate measurement
// - Exponential weighted moving average (EWMA) calculations
// - Per-flow statistics collection
//
// The measurement results are typically attached as tags to packets,
// which can then be used by other modules (such as filters, markers,
// or classifiers) to make decisions based on the measured values.
//
// @see ~IPacketMeter, ~TokenBucketMeter, ~RateMeter
//
simple PacketMeterBase extends PacketFlowBase
{
    parameters:
        displayStringTextFormat = default("metered %p pk (%l)");
        @display("i=block/star");
        @class(PacketMeterBase);
}
