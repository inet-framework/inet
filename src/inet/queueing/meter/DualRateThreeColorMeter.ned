//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

//
// This packet meter module measures the packet flow that is passing through
// using two token buckets. The meter attaches a ~LabelsTag to each packet with
// either green, yellow or red color depending on which token bucket contains
// the required number of tokens for the packet.
//
// The first token bucket is filled with tokens using the committed information
// rate. The maximum number of tokens is determined by the committed burst size.
// The second token bucket is filled with tokens using the excess information
// rate. The maximum number of tokens is determined by the excess burst size.
// Excess tokens from the first token bucket overflow into the second token bucket.
//
// By default, packets consume 1 token per byte.
//
// @see ~SingleRateTwoColorMeter, ~SingleRateThreeColorMeter
// @see ~SingleRateTwoColorClassifier, ~SingleRateThreeColorClassifier, ~DualRateThreeColorClassifier
//
simple DualRateThreeColorMeter extends MultiTokenBucketMeter
{
    parameters:
        double committedInformationRate @unit(bps); // committed information rate
        double excessInformationRate @unit(bps); // excess information rate
        int committedBurstSize @unit(b); // committed burst size
        int excessBurstSize @unit(b); // excess burst size
        int bitsPerToken = default(8); // how many bits are represented by 1 token
        bool colorAware = default(false); // enables color-aware mode where packets can only be demoted
        buckets = [{initialNumTokens: dropUnit(committedBurstSize) / bitsPerToken, maxNumTokens: dropUnit(committedBurstSize) / bitsPerToken, tokenProductionRate: dropUnit(committedInformationRate) / bitsPerToken, label: "green"},
                   {initialNumTokens: dropUnit(excessBurstSize) / bitsPerToken, maxNumTokens: dropUnit(excessBurstSize) / bitsPerToken, tokenProductionRate: dropUnit(excessInformationRate) / bitsPerToken, label: "yellow"}];
        tokenConsumptionPerBit = 1 / bitsPerToken;
        defaultLabel = default("red");
        labelPriority = default(colorAware ? ["green", "yellow", "red"] : []);
}
