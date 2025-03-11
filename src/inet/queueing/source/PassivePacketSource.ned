//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.base.PassivePacketSourceBase;
import inet.queueing.contract.IPassivePacketSource;

//
// Passive packet source that generates packets on demand when pulled by
// connected modules. This module complements ActivePacketSource in the
// queueing framework by providing demand-driven traffic generation.
//
// The module operates by:
// 1. Creating packets according to configured packet length and data parameters
// 2. Providing packets when requested by connected collectors via pull requests
// 3. Controlling packet generation rate through providing interval parameter
//
// Key features:
// - On-demand packet generation based on consumer needs
// - Configurable providing interval to limit packet generation rate
// - Initial providing offset to control when packets become available
// - Support for custom clock modules for synchronized packet generation
// - Ability to generate various packet types and sizes
//
// Typical applications:
// - Modeling responsive traffic sources that only generate traffic when needed
// - Implementing pull-based communication protocols
// - Testing consumer-driven packet processing systems
// - Creating traffic sources for pull-based queueing networks
//
// Timing behavior:
// - The first packet can be provided after initialProvidingOffset
// - Subsequent packets can be provided every providingInterval
// - When providingInterval is 0, any number of packets can be pulled at the same simulation time
// - The providing interval timer starts after a packet is successfully pulled out
//
// @see ~ActivePacketSource, ~EmptyPacketSource, ~PcapFilePacketProducer
//
simple PassivePacketSource extends PassivePacketSourceBase like IPassivePacketSource
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double initialProvidingOffset @unit(s) = default(0s); // Initial duration before which packets are not provided
        volatile double providingInterval @unit(s) = default(0s); // Elapsed time between subsequent packets allowed to be pulled by the connected packet collector, 0 means any number of packets can be pulled at the same simulation time
        bool scheduleForAbsoluteTime = default(true); // When a clock is used, relative means that setting the clock will not affect the simulation time of the event
        @class(PassivePacketSource);
    gates:
        output out @labels(pull);
}
