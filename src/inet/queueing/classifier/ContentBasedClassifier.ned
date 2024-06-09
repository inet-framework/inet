//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.classifier;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module connects one packet producer to multiple packet consumers. It
// can be populated with packets from the connected packet producer. When this
// occurs, the classifier pushes the packet to one of its connected packet
// consumers based on the configured packet filters. The first matching expression
// determines the index of the output gate.
//
simple ContentBasedClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        int defaultGateIndex = default(0); // Default gate index if no matching labels are found
        object packetFilters; // Array of packet filter expressions
        @class(ContentBasedClassifier);
}
