//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

//
// Implements the Single Rate Three Color Marker (srTCM) algorithm as defined in
// RFC 2697. Classifies packets into three categories (typically green, yellow,
// and red) based on their conformance to a committed information rate.
//
// The classifier uses two token buckets operating in series:
// 1. The first bucket (C) is filled at the Committed Information Rate (CIR) and
//    has a capacity equal to the Committed Burst Size (CBS).
// 2. The second bucket (E) is not filled independently but receives excess tokens
//    that overflow from the first bucket. It has a capacity equal to the Excess
//    Burst Size (EBS).
//
// Packet classification works as follows:
// - If the first bucket has enough tokens, the packet is classified as "green" (output 0)
//   and tokens are removed from the first bucket.
// - If the first bucket doesn't have enough tokens but the second bucket does,
//   the packet is classified as "yellow" (output 1) and tokens are removed from
//   the second bucket.
// - If neither bucket has enough tokens, the packet is classified as "red" (output 2).
//
// This classifier is particularly useful for implementing Differentiated Services
// (DiffServ) traffic conditioning, where:
// - Green packets receive the best service level
// - Yellow packets receive a lower service level
// - Red packets may be dropped during congestion
//
// The key difference between this and the DualRateThreeColorClassifier is that
// this classifier uses a single token production rate (CIR), while the dual-rate
// version uses two independent rates.
//
// By default, packets consume 1 token per byte (8 bits per token).
//
// @see ~SingleRateTwoColorMeter, ~SingleRateThreeColorMeter, ~DualRateThreeColorMeter
// @see ~SingleRateTwoColorClassifier, ~DualRateThreeColorClassifier
//
simple SingleRateThreeColorClassifier extends MultiTokenBucketClassifier
{
    parameters:
        double committedInformationRate @unit(bps); // Committed information rate (CIR) - the guaranteed bandwidth
        int committedBurstSize @unit(b); // Committed burst size (CBS) - maximum burst size for guaranteed bandwidth
        int excessBurstSize @unit(b); // Excess burst size (EBS) - additional burst allowance for temporary traffic spikes
        int bitsPerToken = default(8); // How many bits are represented by 1 token (default: 1 token per byte)
        buckets = [{initialNumTokens: dropUnit(committedBurstSize) / bitsPerToken, maxNumTokens: dropUnit(committedBurstSize) / bitsPerToken, tokenProductionRate: dropUnit(committedInformationRate) / bitsPerToken},
                   {initialNumTokens: dropUnit(excessBurstSize) / bitsPerToken, maxNumTokens: dropUnit(excessBurstSize) / bitsPerToken, tokenProductionRate: 0}];
        tokenConsumptionPerBit = 1 / bitsPerToken;
}
