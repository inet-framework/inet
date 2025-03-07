//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Overflow-aware packet classifier that forwards packets to the highest priority
// available consumer. This module implements a simple priority-based load balancing
// mechanism that avoids overflowing downstream queues.
//
// Operation mechanism:
// - Examines connected consumers in order, starting from index 0 (highest priority)
// - Forwards the packet to the first consumer that is not full
// - If all consumers are full, the classifier itself becomes full and cannot accept more packets
//
// The PriorityClassifier is particularly useful for:
// - Load balancing across multiple queues with priority ordering
// - Overflow protection in multi-queue systems
// - Implementing fallback paths for packet processing
// - Simple traffic management without complex classification rules
// - Ensuring packets are not dropped when alternative paths exist
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
