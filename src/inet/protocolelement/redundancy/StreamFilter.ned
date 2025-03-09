//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Filters packets based on their stream identification, allowing only packets
// from specified streams to pass through. Provides a mechanism for stream-based
// traffic selection in redundant communication systems.
//
// Operation:
// - Examines packets for stream identification tags (StreamReq/StreamInd)
// - Compares stream names against configurable filter pattern
// - Passes packets with matching stream names
// - Drops packets with non-matching stream names
// - Supports different filtering modes (request, indication, or both)
//
// @see ~StreamClassifier, ~StreamMerger, ~StreamSplitter
//
simple StreamFilter extends PacketFilterBase like IPacketFilter
{
    parameters:
        string mode @enum("req", "ind", "both") = default("both");
        string streamNameFilter = default("*"); // Filter expression for the stream name
        @class(StreamFilter);
}
