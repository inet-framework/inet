//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

import inet.queueing.contract.IPacketGate;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketShaper;

//
// This module implements a generic packet shaper that combines a queue and a gate
// to control the rate and timing of packet transmissions. It serves as a foundational
// building block for implementing various traffic shaping mechanisms.
//
// The packet shaper consists of two main components:
// 1. A queue that stores and orders packets according to its specific ordering policy
// 2. A gate that controls when packets can be pulled from the queue based on specific criteria
//
// Key features:
// - Flexible architecture that can be customized with different queue and gate implementations
// - Supports various traffic shaping policies through component configuration
// - Can be extended to implement specific shaping algorithms (e.g., token bucket, leaky bucket)
// - Provides a clean separation between packet storage/ordering and transmission timing
//
// The behavior of the shaper is determined by the specific queue and gate implementations used.
// For example:
// - Using a DropTailQueue with a TokenBucketGate creates a token bucket shaper
// - Using a PriorityQueue with a PeriodicGate creates a priority-based periodic shaper
//
// @see ~AsynchronousShaper, ~CreditBasedShaper, ~LeakyBucket, ~TokenBucket, ~TimeAwareShaper
//
module PacketShaper like IPacketShaper
{
    parameters:
        @display("i=block/bucket");
    gates:
        input in;
        output out;
    submodules:
        queue: <default("PacketQueue")> like IPacketQueue {
            @display("p=100,150");
        }
        gate: <default("")> like IPacketGate {
            @display("p=300,150");
        }
    connections:
        in --> queue.in;
        queue.out --> gate.in;
        gate.out --> out;
}
