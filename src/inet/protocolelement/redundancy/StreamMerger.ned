//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Merges packets from redundant streams by eliminating duplicates and optionally
// remapping stream identifiers. Provides a key mechanism for implementing packet-level
// redundancy in communication systems.
//
// Operation:
// - Tracks sequence numbers for each stream
// - Identifies and removes duplicate packets
// - Maintains a configurable buffer of recently seen sequence numbers
// - Optionally remaps input stream names to different output streams
// - Provides the receiving side of redundant stream communication
//
// Particularly useful for:
// - Implementing packet-level redundancy
// - Building fault-tolerant communication systems
// - Creating systems with multiple redundant paths
// - Eliminating duplicate packets in redundant transmissions
// - Supporting seamless stream merging in protocol stacks
//
// @see ~StreamSplitter, ~StreamRelayLayer
//
simple StreamMerger extends PacketFilterBase like IPacketFilter
{
    parameters:
        object mapping @mutable = default({}); // Map of input streams to output streams. An empty string means no stream. For example, {s0: "s1", ...}
        int bufferSize = default(10); // Window size for sequence numbers per stream.
        @class(StreamMerger);
        @display("i=block/join");
}
