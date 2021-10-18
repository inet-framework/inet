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

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module classifies packets to the first output gate if the bucket contains
// enough number of tokens for the packet passing through, otherwise the second
// output gate is used. The amount of required tokens is equal to the packet
// length in bits and is subtracted from the number of stored tokens. New tokens
// are generated with the provided constant rate and they may also be added by
// other modules.
//
simple TokenBucketClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        double initialNumTokens = default(0); // initial number of tokens
        double maxNumTokens = default(inf); // maximum number of tokens
        double tokenProductionRate;
        double tokenConsumptionPerPacket = default(0); // how many tokens are consumed per packet
        double tokenConsumptionPerBit = default(1); // how many tokens are consumed per bit
        string excessTokenModule = default(""); // specifies the module (~ITokenConsumer) that receives the excess tokens that are produced by this module
        displayStringTextFormat = default("contains %n tk\nmetered %p pk (%l)");
        @signal[tokensChanged](type=double);
        @statistic[numTokens](title="number of tokens"; source=tokensChanged; record=vector; unit=tk; interpolationmode=linear);
        @class(TokenBucketClassifier);
}
