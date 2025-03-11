//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

//
// Implements the Single Rate Three Color Marker (srTCM) algorithm as defined in
// RFC 2697. Measures and marks packets with color labels (green, yellow, or red)
// based on their conformance to a committed information rate.
//
// The meter uses two token buckets operating in series:
// 1. The first bucket (C) is filled at the Committed Information Rate (CIR) and
//    has a capacity equal to the Committed Burst Size (CBS).
// 2. The second bucket (E) is not filled independently but receives excess tokens
//    that overflow from the first bucket. It has a capacity equal to the Excess
//    Burst Size (EBS).
//
// Packet marking works as follows:
// - If the first bucket has enough tokens, the packet is marked as "green"
//   and tokens are removed from the first bucket.
// - If the first bucket doesn't have enough tokens but the second bucket does,
//   the packet is marked as "yellow" and tokens are removed from the second bucket.
// - If neither bucket has enough tokens, the packet is marked as "red".
//
// The meter supports both color-blind and color-aware modes:
// - In color-blind mode, the meter ignores any pre-existing color markings.
// - In color-aware mode, packets can only be demoted (green→yellow→red) and
//   never promoted, respecting any pre-existing color markings.
//
// This meter is particularly useful for implementing Differentiated Services
// (DiffServ) traffic conditioning, where:
// - Green packets receive the best service level
// - Yellow packets receive a lower service level
// - Red packets may be dropped during congestion
//
// The key difference between this and the DualRateThreeColorMeter is that
// this meter uses a single token production rate (CIR), while the dual-rate
// version uses two independent rates.
//
// By default, packets consume 1 token per byte (8 bits per token).
//
// @see ~SingleRateTwoColorMeter, ~DualRateThreeColorMeter
// @see ~SingleRateTwoColorClassifier, ~SingleRateThreeColorClassifier, ~DualRateThreeColorClassifier
//
simple SingleRateThreeColorMeter extends MultiTokenBucketMeter
{
    parameters:
        double committedInformationRate @unit(bps); // Committed information rate (CIR) - the guaranteed bandwidth
        int committedBurstSize @unit(b); // Committed burst size (CBS) - maximum burst size for guaranteed bandwidth
        int excessBurstSize @unit(b); // Excess burst size (EBS) - additional burst allowance for temporary traffic spikes
        int bitsPerToken = default(8); // How many bits are represented by 1 token (default: 1 token per byte)
        bool colorAware = default(false); // When true, enables color-aware mode where packets can only be demoted (green→yellow→red)
        buckets = [{initialNumTokens: dropUnit(committedBurstSize) / bitsPerToken, maxNumTokens: dropUnit(committedBurstSize) / bitsPerToken, tokenProductionRate: dropUnit(committedInformationRate) / bitsPerToken, label: "green"},
                   {initialNumTokens: dropUnit(excessBurstSize) / bitsPerToken, maxNumTokens: dropUnit(excessBurstSize) / bitsPerToken, tokenProductionRate: 0, label: "yellow"}];
        tokenConsumptionPerBit = 1 / bitsPerToken;
        defaultLabel = default("red");
        labelPriority = default(colorAware ? ["green", "yellow", "red"] : []);
}
