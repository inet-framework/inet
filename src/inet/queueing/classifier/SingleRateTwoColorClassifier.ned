//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

//
// This packet classifier implements the Single Rate Two Color Marker (srTCM) algorithm,
// a simplified version of the traffic conditioning mechanisms defined in RFC 2697.
// It classifies packets into two categories (typically green and red) based on
// their conformance to a committed information rate.
//
// The classifier uses a single token bucket that is filled at the Committed
// Information Rate (CIR) and has a capacity equal to the Committed Burst Size (CBS).
//
// Packet classification works as follows:
// - If the token bucket has enough tokens, the packet is classified as "conforming" (output 0)
//   and tokens are removed from the bucket.
// - If the token bucket doesn't have enough tokens, the packet is classified as
//   "non-conforming" (output 1) and no tokens are removed.
//
// The single-rate two-color marker is simpler than the three-color variants
// (SingleRateThreeColorClassifier and DualRateThreeColorClassifier) as it only
// distinguishes between two traffic categories rather than three.
//
// By default, packets consume 1 token per byte (8 bits per token).
//
// @see ~SingleRateTwoColorMeter, ~SingleRateThreeColorMeter, ~DualRateThreeColorMeter
// @see ~SingleRateThreeColorClassifier, ~DualRateThreeColorClassifier
//
simple SingleRateTwoColorClassifier extends TokenBucketClassifier
{
    parameters:
        double committedInformationRate @unit(bps); // Committed information rate (CIR) - the guaranteed bandwidth
        int committedBurstSize @unit(b); // Committed burst size (CBS) - maximum burst size for guaranteed bandwidth
        int bitsPerToken = default(8); // How many bits are represented by 1 token (default: 1 token per byte)
        initialNumTokens = default(maxNumTokens);
        maxNumTokens = dropUnit(committedBurstSize) / bitsPerToken;
        tokenProductionRate = dropUnit(committedInformationRate) / bitsPerToken;
        tokenConsumptionPerBit = 1 / bitsPerToken;
}
