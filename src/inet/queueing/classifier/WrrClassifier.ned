//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module implements weighted round-robin classifier.
//
// There is a weight associated with each output gate.
// Initially each output gate gets as many tokens as their
// weight. When a packet is pushed then the first output
// gate is selected that has a packet and enough tokens,
// and one token is consumed. When there is not enough
// tokens to classify a packet, then each token bucket
// is filled up.
//
simple WrrClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string weights;
        @class(WrrClassifier);
}
