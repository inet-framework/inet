//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// This module classifies packets based on the stream they are part of. The stream
// is determined by a StreamReq or a StreamInd or both.
//
simple StreamClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string mode @enum("req", "ind", "both") = default("both");
        object mapping = default({}); // maps stream name to gate index
        int gateIndexOffset = default(0); // specify value to be added to the mapping output gate index
        int defaultGateIndex = default(0); // default gate index if no matching streams are found
        @class(StreamClassifier);
}
