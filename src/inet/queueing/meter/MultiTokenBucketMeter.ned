//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;

//
// Multi-token bucket meter that uses multiple token buckets to label packets
// based on their conformance to different traffic profiles.
//
// The meter operates by:
// - Maintaining multiple token buckets, each with its own parameters
// - Checking each bucket in sequence for the required number of tokens
// - Labeling the packet according to the first bucket that has enough tokens
// - If no bucket has enough tokens, applying the default label (if specified)
//
// Key features:
// - Supports an arbitrary number of token buckets
// - Each bucket can have different token production rates and capacities
// - Configurable token consumption per packet or per bit
// - Optional token overflow between buckets
// - Comprehensive statistics collection for each bucket
// - Support for color-aware mode through the labelPriority parameter
//
// The color-aware mode works as follows:
// - When labelPriority is specified, packets can only be promoted in the order defined
// - For example, with labelPriority=["green", "yellow", "red"], a packet already
//   marked as "yellow" can only be marked as "yellow" or "red", never "green"
//
// This meter is particularly useful for:
// - Implementing complex traffic conditioning mechanisms
// - Multi-level traffic marking for Differentiated Services (DiffServ)
// - Quality of Service implementations with multiple service levels
// - Hierarchical bandwidth allocation schemes
// - Traffic policing with multiple conformance levels
//
// The buckets parameter allows detailed configuration of each token bucket,
// including initial token count, maximum capacity, token production rate,
// and the label to apply when that bucket has enough tokens.
//
// @see ~TokenBucketMeter, ~SingleRateThreeColorMeter, ~DualRateThreeColorMeter
//
simple MultiTokenBucketMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        object buckets; // Array of objects specifying the token buckets
                        // example: [{initialNumTokens: 100, maxNumTokens: 1000, tokenProductionRate: 10, label: "green", excessTokenModule: ".other"}]
                        // - initialNumTokens: Initial number of tokens in the bucket
                        // - maxNumTokens: Maximum number of tokens the bucket can hold
                        // - tokenProductionRate: Rate at which tokens are generated (tokens/sec)
                        // - label: Label to attach to packets that conform to this bucket
                        // - excessTokenModule: Optional path to another bucket that receives excess tokens
        double tokenConsumptionPerPacket = default(0); // Fixed number of tokens consumed per packet (regardless of size)
        double tokenConsumptionPerBit = default(1); // Number of tokens consumed per bit of packet length
        string defaultLabel = default(""); // Label attached to packets if no bucket has enough tokens (empty string means no label)
        object labelPriority = default([]); // List of labels in order of priority for color-aware mode (empty list means color-blind mode)
        displayStringTextFormat = default("contains %n tk\nmetered %p pk (%l)");
        @signal[tokensChanged](type=double);
        @statistic[numTokens](title="number of tokens"; source=demux(tokensChanged); record=vector; unit=tk; interpolationmode=linear);
        @class(MultiTokenBucketMeter);
}
