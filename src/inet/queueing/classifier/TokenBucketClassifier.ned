//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Basic token bucket classifier that forms the foundation for various traffic
// conditioning mechanisms in the queueing framework.
//
// The token bucket algorithm works by:
// - Maintaining a bucket of tokens that fills at a constant rate
// - Checking if enough tokens are available when a packet arrives
// - Consuming tokens equal to the packet size when forwarding a packet
//
// Packet classification works as follows:
// - If the bucket contains enough tokens for the packet, the packet is classified
//   as "conforming" (output 0) and tokens are removed from the bucket.
// - If the bucket doesn't have enough tokens, the packet is classified as
//   "non-conforming" (output 1) and no tokens are removed.
//
// @see ~SingleRateTwoColorClassifier, ~SingleRateThreeColorClassifier, ~DualRateThreeColorClassifier
//
simple TokenBucketClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        double initialNumTokens = default(0); // Initial number of tokens in the bucket
        double maxNumTokens = default(inf); // Maximum number of tokens the bucket can hold (bucket capacity)
        double tokenProductionRate; // Rate at which new tokens are generated (tokens per second)
        double tokenConsumptionPerPacket = default(0); // Fixed number of tokens consumed per packet (regardless of size)
        double tokenConsumptionPerBit = default(1); // Number of tokens consumed per bit of packet length
        string excessTokenModule = default(""); // Specifies the module (~ITokenStorage) that receives the excess tokens that are produced when this bucket is full
        displayStringTextFormat = default("contains %n tk\nmetered %p pk (%l)");
        @signal[tokensChanged](type=double);
        @statistic[numTokens](title="number of tokens"; source=tokensChanged; record=vector; unit=tk; interpolationmode=linear);
        @class(TokenBucketClassifier);
}
