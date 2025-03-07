//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.sink;

import inet.queueing.base.ActivePacketSinkBase;
import inet.queueing.contract.IActivePacketSink;

//
// Active packet sink that pulls packets at regular intervals from connected
// modules and consumes them. This module is a fundamental traffic consumption
// component in the queueing framework.
//
// The module operates by:
// 1. Actively requesting packets from connected providers at specified intervals
// 2. Pulling packets through the input gate using the pull mechanism
// 3. Processing and consuming the received packets (counting and deleting them)
// 4. Respecting backpressure conditions from upstream modules
//
// Key features:
// - Configurable collection interval for precise traffic consumption patterns
// - Initial collection offset to control when the first packet is collected
// - Backpressure awareness to prevent pulling when providers cannot supply packets
// - Support for custom clock modules for synchronized packet collection
// - Comprehensive packet statistics collection
//
// Typical applications:
// - Modeling traffic sinks in network simulations
// - Testing network elements under specific consumption patterns
// - Measuring throughput and latency in queueing networks
// - Implementing pull-based traffic management systems
//
// Timing behavior:
// - The first packet is collected after initialCollectionOffset (or immediately if negative)
// - Subsequent packets are collected every collectionInterval
// - If backpressure is present, packet collection is delayed until backpressure is removed
// - The collection interval timer always starts after a packet is successfully pulled in
//
// Invariants:
// - Two subsequent packets are always pulled in at least `collectionInterval` time apart
// - Backpressure prevents packets from being pulled in
//
// @see ~PassivePacketSink, ~FullPacketSink, ~PcapFilePacketConsumer
//
simple ActivePacketSink extends ActivePacketSinkBase like IActivePacketSink
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double initialCollectionOffset @unit(s) = default(-1s); // Initial duration before which packets are not collected, negative value means: try the first collection in initialize()
        volatile double collectionInterval @unit(s); // Elapsed time between subsequent packets pulled from the connected packet provider
        bool scheduleForAbsoluteTime = default(true); // When a clock is used, relative means that setting the clock will not affect the simulation time of the event
        @class(ActivePacketSink);
    gates:
        input in @labels(pull);
}
