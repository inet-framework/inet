//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

//
// Implements the Single Rate Two Color Marker (srTCM) algorithm, a simplified
// version of the traffic conditioning mechanisms defined in RFC 2697. Measures
// and marks packets with color labels (green or red) based on their conformance
// to a committed information rate.
//
// The meter uses a single token bucket that is filled at the Committed
// Information Rate (CIR) and has a capacity equal to the Committed Burst Size (CBS).
//
// Packet marking works as follows:
// - If the token bucket has enough tokens, the packet is marked as "green"
//   and tokens are removed from the bucket.
// - If the token bucket doesn't have enough tokens, the packet is marked as "red"
//   and no tokens are removed.
//
// The meter supports both color-blind and color-aware modes:
// - In color-blind mode, the meter ignores any pre-existing color markings.
// - In color-aware mode, packets can only be demoted (green→red) and
//   never promoted, respecting any pre-existing color markings.
//
// The single-rate two-color marker is simpler than the three-color variants
// (SingleRateThreeColorMeter and DualRateThreeColorMeter) as it only
// distinguishes between two traffic categories rather than three.
//
// By default, packets consume 1 token per byte (8 bits per token).
//
// @see ~SingleRateThreeColorMeter, ~DualRateThreeColorMeter
// @see ~SingleRateTwoColorClassifier, ~SingleRateThreeColorClassifier, ~DualRateThreeColorClassifier
//
simple SingleRateTwoColorMeter extends TokenBucketMeter
{
    parameters:
        double committedInformationRate @unit(bps); // Committed information rate (CIR) - the guaranteed bandwidth
        int committedBurstSize @unit(b); // Committed burst size (CBS) - maximum burst size for guaranteed bandwidth
        int bitsPerToken = default(8); // How many bits are represented by 1 token (default: 1 token per byte)
        bool colorAware = default(false); // When true, enables color-aware mode where packets can only be demoted (green→red)
        initialNumTokens = default(maxNumTokens);
        maxNumTokens = dropUnit(committedBurstSize) / bitsPerToken;
        tokenProductionRate = dropUnit(committedInformationRate) / bitsPerToken;
        tokenConsumptionPerBit = 1 / bitsPerToken;
        label = default("green");
        defaultLabel = default("red");
        labelPriority = default(colorAware ? ["green", "red"] : []);
}
