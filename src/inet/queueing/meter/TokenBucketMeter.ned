//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.meter;

import inet.queueing.base.PacketMeterBase;
import inet.queueing.contract.IPacketMeter;

//
// This module labels each packet if the token bucket contains enough tokens for
// the packet that is passing through. The amount of required tokens is equal to
// the packet length in bits and is subtracted from the number of stored tokens.
// New tokens are generated with the provided constant rate and they may also be
// added by other modules.
//
simple TokenBucketMeter extends PacketMeterBase like IPacketMeter
{
    parameters:
        double initialNumTokens = default(0); // initial number of tokens in the bucket
        double maxNumTokens = default(inf); // maximum number of tokens in the bucket
        double tokenProductionRate; // the rate of internal token production per second
        double tokenConsumptionPerPacket = default(0); // how many tokens are consumed per packet
        double tokenConsumptionPerBit = default(1); // how many tokens are consumed per bit
        string label; // the label that is attached to packets if there are enough tokens in the bucket
        string defaultLabel = default(""); // the label that is attached to packets if there are not enouth tokens in the bucket, no label by default
        object labelPriority = default([]); // the list of labels in the order of priority, if specified packets can only be promoted in this order
        string excessTokenModule = default(""); // specifies the module (~ITokenConsumer) that receives the excess tokens that are produced by this module when overflown
        displayStringTextFormat = default("contains %n tk\nmetered %p pk (%l)");
        @signal[tokensChanged](type=double);
        @statistic[numTokens](title="number of tokens"; source=tokensChanged; record=vector; unit=tk; interpolationmode=linear);
        @class(TokenBucketMeter);
}
