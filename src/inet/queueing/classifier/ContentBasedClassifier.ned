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
// can be pushed with packets from the connected packet producer. When this
// happens, the classifier pushes the packet to one of its connected packet
// consumers based on the configured packet filters. The first matching expression
// determines the index of the output gate.
//
simple ContentBasedClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        int defaultGateIndex = default(0); // default gate index if no matching labels are found
        object packetFilters; // array of packet filter expressions
        @class(ContentBasedClassifier);
}
