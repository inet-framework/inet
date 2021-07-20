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
