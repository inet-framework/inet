//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// Classifies packets based on their stream identification and directs them to
// different output gates. Provides a mechanism for stream-based traffic separation
// in redundant communication systems.
//
// Operation:
// - Examines packets for stream identification tags (StreamReq/StreamInd)
// - Maps stream names to output gate indices using configurable mapping
// - Routes packets to appropriate output gates based on stream identity
// - Supports different classification modes (request, indication, or both)
// - Falls back to default gate when no matching stream is found
//
// @see ~StreamFilter, ~StreamMerger, ~StreamSplitter
//
simple StreamClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        string mode @enum("req", "ind", "both") = default("both");
        object mapping = default({}); // Maps stream name to gate index
        int gateIndexOffset = default(0); // Specify the value to be added to the mapping output gate index
        int defaultGateIndex = default(0); // Default gate index if no matching streams are found
        @class(StreamClassifier);
}
