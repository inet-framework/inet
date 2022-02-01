//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

//
// This packet meter module measures the packet flow that is passing through
// using one token bucket. The meter attaches a ~LabelsTag to each packet with
// either green or red color depending on whether the token bucket contains the
// required number of tokens for the packet.
//
// By default, packets consume 1 token per byte.
//
// @see ~SingleRateThreeColorMeter, ~DualRateThreeColorMeter
// @see ~SingleRateTwoColorClassifier, ~SingleRateThreeColorClassifier, ~DualRateThreeColorClassifier
//
simple SingleRateTwoColorMeter extends TokenBucketMeter
{
    parameters:
        double committedInformationRate @unit(bps); // committed information rate
        int committedBurstSize @unit(b); // committed burst size
        int bitsPerToken = default(8); // how many bits are represented by 1 token
        bool colorAware = default(false); // enables color-aware mode where packets can only be demoted
        initialNumTokens = default(maxNumTokens);
        maxNumTokens = dropUnit(committedBurstSize) / bitsPerToken;
        tokenProductionRate = dropUnit(committedInformationRate) / bitsPerToken;
        tokenConsumptionPerBit = 1 / bitsPerToken;
        label = default("green");
        defaultLabel = default("red");
        labelPriority = default(colorAware ? ["green", "red"] : []);
}
