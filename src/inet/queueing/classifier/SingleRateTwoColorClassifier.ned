//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

//
// This packet classifier module classifies packets using one token bucket. Each
// packet is classified depending on whether the token bucket contains the required
// number of tokens for the packet.
//
// The token bucket is filled with tokens using the committed information rate.
// The maximum number of tokens is determined by the committed burst size.
//
// By default, packets consume 1 token per byte.
//
// @see ~SingleRateTwoColorMeter, ~SingleRateThreeColorMeter, ~DualRateThreeColorMeter
// @see ~SingleRateThreeColorClassifier, ~DualRateThreeColorClassifier
//
simple SingleRateTwoColorClassifier extends TokenBucketClassifier
{
    parameters:
        double committedInformationRate @unit(bps); // committed information rate
        int committedBurstSize @unit(b); // committed burst size
        int bitsPerToken = default(8); // how many bits are represented by 1 token
        initialNumTokens = default(maxNumTokens);
        maxNumTokens = dropUnit(committedBurstSize) / bitsPerToken;
        tokenProductionRate = dropUnit(committedInformationRate) / bitsPerToken;
        tokenConsumptionPerBit = 1 / bitsPerToken;
}
