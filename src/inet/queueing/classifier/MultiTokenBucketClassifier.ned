//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module classifies packets into the first bucket that contains the required
// number of tokens. The packet is forwarded on the output gate that corresponds to
// the selected bucket.
//
simple MultiTokenBucketClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        object buckets; // array of objects specifying the token buckets
                        // example: [{initialNumTokens: 100, maxNumTokens: 1000, tokenProductionRate: 10, excessTokenModule: ".other"}]
        double tokenConsumptionPerPacket = default(0); // how many tokens are consumed per packet
        double tokenConsumptionPerBit = default(1); // how many tokens are consumed per bit
        displayStringTextFormat = default("contains %n tk\nmetered %p pk (%l)");
        @signal[tokensChanged](type=double);
        @statistic[numTokens](title="number of tokens"; source=demux(tokensChanged); record=vector; unit=tk; interpolationmode=linear);
        @class(MultiTokenBucketClassifier);
}
