//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

//
// This packet classifier module classifies packets using two token buckets.
// Each packet is classified depending on which token bucket is the first one
// that contains the required number of tokens for the packet.
//
// The first token bucket is filled with tokens using the committed information
// rate. The maximum number of tokens is determined by the committed burst size.
// The second token bucket is filled with tokens using the excess information
// rate. The maximum number of tokens is determined by the excess burst size.
// Excess tokens from the first token bucket overflow into the second token bucket.
//
// By default, packets consume 1 token per byte.
//
// @see ~SingleRateTwoColorMeter, ~SingleRateThreeColorMeter, ~DualRateThreeColorMeter
// @see ~SingleRateTwoColorClassifier, ~SingleRateThreeColorClassifier
//
simple DualRateThreeColorClassifier extends MultiTokenBucketClassifier
{
    parameters:
        double committedInformationRate @unit(bps); // committed information rate
        double excessInformationRate @unit(bps); // excess information rate
        int committedBurstSize @unit(b); // committed burst size
        int excessBurstSize @unit(b); // excess burst size
        int bitsPerToken = default(8); // how many bits are represented by 1 token
        buckets = [{initialNumTokens: dropUnit(committedBurstSize) / bitsPerToken, maxNumTokens: dropUnit(committedBurstSize) / bitsPerToken, tokenProductionRate: dropUnit(committedInformationRate) / bitsPerToken},
                   {initialNumTokens: dropUnit(excessBurstSize) / bitsPerToken, maxNumTokens: dropUnit(excessBurstSize) / bitsPerToken, tokenProductionRate: dropUnit(excessInformationRate) / bitsPerToken}];
        tokenConsumptionPerBit = 1 / bitsPerToken;
}
