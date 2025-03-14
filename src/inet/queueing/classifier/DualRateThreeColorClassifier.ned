//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

//
// Classifies packets into three categories (typically green, yellow,
// and red) based on their conformance to committed and excess information rates.
//
// The classifier uses two token buckets operating in series:
// 1. The first bucket (C) is filled at the Committed Information Rate (CIR) and
//    has a capacity equal to the Committed Burst Size (CBS).
// 2. The second bucket (E) is filled at the Excess Information Rate (EIR) and
//    has a capacity equal to the Excess Burst Size (EBS).
//
// Packet classification works as follows:
// - If the first bucket has enough tokens, the packet is classified as "green" (output 0)
//   and tokens are removed from the first bucket.
// - If the first bucket doesn't have enough tokens but the second bucket does,
//   the packet is classified as "yellow" (output 1) and tokens are removed from
//   the second bucket.
// - If neither bucket has enough tokens, the packet is classified as "red" (output 2).
//
// By default, packets consume 1 token per byte (8 bits per token).
//
// @see ~SingleRateTwoColorMeter, ~SingleRateThreeColorMeter, ~DualRateThreeColorMeter
// @see ~SingleRateTwoColorClassifier, ~SingleRateThreeColorClassifier
//
simple DualRateThreeColorClassifier extends MultiTokenBucketClassifier
{
    parameters:
        double committedInformationRate @unit(bps); // Committed information rate (CIR) - the guaranteed bandwidth
        double excessInformationRate @unit(bps); // Excess information rate (EIR) - the additional bandwidth that may be available
        int committedBurstSize @unit(b); // Committed burst size (CBS) - maximum burst size for guaranteed bandwidth
        int excessBurstSize @unit(b); // Excess burst size (EBS) - maximum burst size for excess bandwidth
        int bitsPerToken = default(8); // How many bits are represented by 1 token (default: 1 token per byte)
        buckets = [{initialNumTokens: dropUnit(committedBurstSize) / bitsPerToken, maxNumTokens: dropUnit(committedBurstSize) / bitsPerToken, tokenProductionRate: dropUnit(committedInformationRate) / bitsPerToken},
                   {initialNumTokens: dropUnit(excessBurstSize) / bitsPerToken, maxNumTokens: dropUnit(excessBurstSize) / bitsPerToken, tokenProductionRate: dropUnit(excessInformationRate) / bitsPerToken}];
        tokenConsumptionPerBit = 1 / bitsPerToken;
}
