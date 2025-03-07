//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This packet classifier implements a multi-token bucket algorithm that uses
// multiple token buckets to classify packets into different categories based
// on their conformance to various traffic profiles.
//
// The classifier operates by:
// - Maintaining multiple token buckets, each with its own parameters
// - Checking each bucket in sequence for the required number of tokens
// - Classifying the packet to the first bucket that has enough tokens
// - Forwarding the packet to the output gate corresponding to that bucket
//
// Key features:
// - Supports an arbitrary number of token buckets
// - Each bucket can have different token production rates and capacities
// - Configurable token consumption per packet or per bit
// - Optional token overflow between buckets
// - Comprehensive statistics collection for each bucket
//
// The buckets parameter allows detailed configuration of each token bucket,
// including initial token count, maximum capacity, and token production rate.
// Tokens can optionally overflow from one bucket to another through the
// excessTokenModule parameter.
//
// @see ~TokenBucketClassifier, ~SingleRateThreeColorClassifier, ~DualRateThreeColorClassifier
//
simple MultiTokenBucketClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        object buckets; // Array of objects specifying the token buckets
                        // example: [{initialNumTokens: 100, maxNumTokens: 1000, tokenProductionRate: 10, excessTokenModule: ".other"}]
        double tokenConsumptionPerPacket = default(0); // How many tokens are consumed per packet
        double tokenConsumptionPerBit = default(1); // How many tokens are consumed per bit
        displayStringTextFormat = default("contains %n tk\nmetered %p pk (%l)");
        @signal[tokensChanged](type=double);
        @statistic[numTokens](title="number of tokens"; source=demux(tokensChanged); record=vector; unit=tk; interpolationmode=linear);
        @class(MultiTokenBucketClassifier);
}
