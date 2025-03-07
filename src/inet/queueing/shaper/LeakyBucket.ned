//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;
import inet.queueing.contract.ITrafficConditioner;

//
// Implements the classic leaky bucket algorithm for traffic shaping and rate limiting.
// Unlike the token bucket algorithm which allows bursts up to a certain size, the leaky
// bucket strictly enforces a constant output rate regardless of input traffic patterns.
//
// The leaky bucket algorithm works as follows:
// - Incoming packets are stored in a queue (the "bucket")
// - Packets are removed from the queue at a constant rate (the "leak")
// - If the queue is full when a new packet arrives, the packet is typically dropped
//
// Key features:
// - Enforces a strict constant output rate
// - Smooths out traffic bursts
// - Provides predictable delay characteristics
// - Simple to implement and understand
// - Configurable queue type for different drop policies
//
// This implementation consists of:
// - A configurable queue that stores incoming packets
// - A server that processes packets at a constant rate
//
// @see ~TokenBucket, ~CreditBasedShaper
//
module LeakyBucket like ITrafficConditioner
{
    parameters:
        @display("i=block/buffer");
    gates:
        input in @labels(push);
        output out @labels(pull);
    submodules:
        queue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=125,120");
        }
        server: <default("PacketServer")> like IPacketServer {
            parameters:
                @display("p=325,120");
        }
    connections:
        in --> { @display("m=w"); } --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=e"); } --> out;
}
