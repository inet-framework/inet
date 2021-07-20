//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
// Excess tokens from the first token bucket overflow into the second token bucket.
//
// By default, packets consume 1 token per byte.
//
// @see ~SingleRateTwoColorMeter, ~DualRateThreeColorMeter
// @see ~SingleRateTwoColorClassifier, ~SingleRateThreeColorClassifier, ~DualRateThreeColorClassifier
//
simple SingleRateThreeColorMeter extends MultiTokenBucketMeter
{
    parameters:
        double committedInformationRate @unit(bps); // committed information rate
        int committedBurstSize @unit(b); // committed burst size
        int excessBurstSize @unit(b); // excess burst size
        int bitsPerToken = default(8); // how many bits are represented by 1 token
        bool colorAware = default(false); // enables color-aware mode where packets can only be demoted
        buckets = [{initialNumTokens: dropUnit(committedBurstSize) / bitsPerToken, maxNumTokens: dropUnit(committedBurstSize) / bitsPerToken, tokenProductionRate: dropUnit(committedInformationRate) / bitsPerToken, label: "green"},
                   {initialNumTokens: dropUnit(excessBurstSize) / bitsPerToken, maxNumTokens: dropUnit(excessBurstSize) / bitsPerToken, tokenProductionRate: 0, label: "yellow"}];
        tokenConsumptionPerBit = 1 / bitsPerToken;
        defaultLabel = default("red");
        labelPriority = default(colorAware ? ["green", "yellow", "red"] : []);
}
