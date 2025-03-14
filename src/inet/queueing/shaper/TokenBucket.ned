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
// Implements the classic token bucket algorithm for traffic shaping and rate limiting.
// Unlike the leaky bucket which enforces a strict constant output rate, the token
// bucket allows for bursty traffic up to a configurable maximum burst size.
//
// The token bucket algorithm works as follows:
// - Tokens are generated at a constant rate and stored in a bucket
// - The bucket has a maximum capacity (limiting the maximum burst size)
// - When a packet arrives, it can only be transmitted if enough tokens are available
// - Each transmitted packet consumes tokens proportional to its size
// - If insufficient tokens are available, the packet is queued until enough tokens accumulate
//
// Key features:
// - Allows for bursty traffic while maintaining long-term rate limits
// - Configurable token generation rate to control average throughput
// - Configurable bucket size to control maximum burst size
// - Flexible queue type for different packet handling policies
// - Token-based server for precise rate control
//
// This implementation consists of:
// - A configurable queue that stores packets when insufficient tokens are available
// - A token-based server that controls packet transmission based on token availability
//
// @see ~LeakyBucket, ~TokenBucketMeter, ~TokenBucketClassifier
//
module TokenBucket like ITrafficConditioner
{
    parameters:
        @display("i=block/buffer");
    gates:
        input in @labels(push);
        output out @labels(pull);
    submodules:
        queue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=150,120");
        }
        server: <default("TokenBasedServer")> like IPacketServer {
            parameters:
                @display("p=350,120");
        }
    connections:
        in --> { @display("m=w"); } --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=e"); } --> out;
}
