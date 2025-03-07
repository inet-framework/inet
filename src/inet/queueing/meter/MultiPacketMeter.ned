//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.contract.IPacketMeter;

//
// This module implements a composite packet meter that chains multiple packet
// meters together in series. Each packet passes through all the meters in sequence,
// allowing for multi-stage packet metering and tagging.
//
// The module creates a pipeline of packet meters where:
// - Packets enter through the input gate
// - Pass through each meter in the chain in order
// - Exit through the output gate
//
// Key features:
// - Supports an arbitrary number of packet meters in sequence
// - Each meter can perform different measurements or apply different tags
// - Allows for complex, multi-criteria packet metering
// - Compatible with any module that implements the IPacketMeter interface
// - Preserves all tags applied by each meter in the chain
//
// @see ~TokenBucketMeter, ~ExponentialRateMeter, ~SlidingWindowRateMeter
//
module MultiPacketMeter like IPacketMeter
{
    parameters:
        int numMeters; // Number of packet meters in the chain
        @display("i=block/star");
    gates:
        input in @labels(send,push,pull,pass,stream);
        output out @labels(send,push,pull,pass,stream);
    submodules:
        meter[numMeters]: <> like IPacketMeter {
            @display("p=150,150,row,150");
        }
    connections:
        in --> { @display("m=w"); } --> meter[0].in;
        for i=1..numMeters-1 {
            meter[i - 1].out --> meter[i].in;
        }
        meter[numMeters - 1].out --> { @display("m=e"); } --> out;
}
