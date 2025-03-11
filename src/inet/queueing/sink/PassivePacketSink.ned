//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.sink;

import inet.queueing.base.PassivePacketSinkBase;
import inet.queueing.contract.IPassivePacketSink;

//
// Passive packet sink that receives packets pushed to it by connected
// modules and consumes them. This module complements ActivePacketSink
// in the queueing framework by providing push-based traffic consumption.
//
// The module operates by:
// 1. Passively receiving packets pushed to its input gate
// 2. Processing and consuming the received packets (counting and deleting them)
// 3. Optionally controlling the rate at which packets can be consumed
//
// Key features:
// - Push-based operation where producers determine when packets are sent
// - Configurable consumption interval to limit packet processing rate
// - Initial consumption offset to control when packets can first be consumed
// - Support for custom clock modules for synchronized packet consumption
// - Comprehensive packet statistics collection
//
// Typical applications:
// - Modeling traffic endpoints in push-based communication systems
// - Implementing network protocol termination points
// - Testing producer modules under various consumption constraints
// - Creating traffic sinks for push-based queueing networks
//
// Timing behavior:
// - The first packet can be consumed after initialConsumptionOffset
// - Subsequent packets can be consumed every consumptionInterval
// - When consumptionInterval is 0, any number of packets can be pushed at the same simulation time
// - The consumption interval timer starts after a packet is successfully consumed
//
// @see ~ActivePacketSink, ~FullPacketSink, ~PcapFilePacketConsumer
//
simple PassivePacketSink extends PassivePacketSinkBase like IPassivePacketSink
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double initialConsumptionOffset @unit(s) = default(0s); // Initial duration before which packets are not consumed
        volatile double consumptionInterval @unit(s) = default(0s); // Elapsed time between subsequent packets allowed to be pushed by the connected packet producer, 0 means any number of packets can be pushed at the same simulation time
        bool scheduleForAbsoluteTime = default(true); // When a clock is used, "relative" means that setting the clock will not affect the simulation time of the event
        @class(PassivePacketSink);
    gates:
        input in @labels(push);
}
