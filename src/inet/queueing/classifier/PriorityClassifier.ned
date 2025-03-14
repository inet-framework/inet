//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Back-pressure aware packet classifier that forwards packets to the first
// accepting output.
//
// Operation mechanism:
// - Examines connected consumers in order, starting from index 0 (highest priority)
// - Forwards the packet to the first consumer that doesn't provide back-pressure
// - If no consumers are accepting, the classifier itself provides back-pressure
//
// Unlike other classifiers that use packet attributes to determine the output,
// this classifier bases its decision solely on the state of the connected consumers.
//
// @see ~PacketClassifier, ~ContentBasedClassifier, ~LabelClassifier
//
simple PriorityClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        @class(PriorityClassifier);
}
