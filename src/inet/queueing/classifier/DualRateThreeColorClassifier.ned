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

package inet.queueing.classifier;

simple DualRateThreeColorClassifier extends MultiTokenBucketClassifier
{
    parameters:
        double cir @unit(bps); // committed information rate
        double eir @unit(bps); // excess information rate
        int cbs @unit(b); // committed burst size
        int ebs @unit(b); // excess burst size
        int bitsPerToken = default(8); // how many bits are represented by 1 token
        buckets = [{initialNumTokens: dropUnit(cbs) / bitsPerToken, maxNumTokens: dropUnit(cbs) / bitsPerToken, tokenProductionRate: dropUnit(cir) / bitsPerToken},
                   {initialNumTokens: dropUnit(ebs) / bitsPerToken, maxNumTokens: dropUnit(ebs) / bitsPerToken, tokenProductionRate: dropUnit(eir) / bitsPerToken}];
        tokenConsumptionPerBit = 1 / bitsPerToken;
}
