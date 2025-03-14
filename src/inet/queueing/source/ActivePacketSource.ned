//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.base.ActivePacketSourceBase;
import inet.queueing.contract.IActivePacketSource;

//
// Active packet source that generates packets at regular intervals and pushes
// them to the connected module. This module is a fundamental traffic generation
// component in the queueing framework.
//
// The module operates by:
// 1. Creating packets according to configured packet length and data parameters
// 2. Pushing packets to the connected consumer at specified time intervals
// 3. Respecting backpressure from downstream modules
//
// Key features:
// - Configurable production interval for precise traffic pattern generation
// - Initial production offset to control when the first packet is generated
// - Backpressure awareness to prevent packet loss when consumers are congested
// - Support for custom clock modules for synchronized packet generation
// - Ability to generate various packet types and sizes
//
// Timing behavior:
// - The first packet is generated after initialProductionOffset (or immediately if negative)
// - Subsequent packets are generated every productionInterval
// - If backpressure is present, packet generation is delayed until backpressure is removed
// - The production interval timer always starts after a packet is successfully pushed out
//
// Invariants:
// - Two subsequent packets are always pushed out at least `productionInterval` time apart
// - Backpressure prevents packets from being pushed out
//
// @see ~PassivePacketSource, ~EmptyPacketSource, ~PcapFilePacketProducer
//
simple ActivePacketSource extends ActivePacketSourceBase like IActivePacketSource
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double initialProductionOffset @mutable @unit(s) = default(-1s); // Initial duration before which packets are not produced. When negative, then produce packet in initialize()
        volatile double productionInterval @unit(s); // Elapsed time between subsequent packets pushed to the connected packet consumer
        bool scheduleForAbsoluteTime = default(true); // When a clock is used, "relative" means that setting the clock will not affect the simulation time of the event
        @class(ActivePacketSource);
    gates:
        output out @labels(push);
}
