//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketPusher;

//
// Duplicates packets into multiple streams for redundant transmission. Provides
// a key mechanism for implementing packet-level redundancy in communication systems.
//
// Operation:
// - Examines packets for stream identification tags
// - Creates copies of packets based on configurable stream mapping
// - Assigns different stream identifiers to each copy
// - Forwards redundant copies to the same output gate
// - Provides the sending side of redundant stream communication
//
// Particularly useful for:
// - Implementing packet-level redundancy
// - Building fault-tolerant communication systems
// - Creating systems with multiple redundant paths
// - Supporting seamless packet duplication in protocol stacks
// - Implementing Parallel Redundancy Protocol (PRP) or similar concepts
//
// @see ~StreamMerger, ~StreamRelayLayer
//
simple StreamSplitter extends PacketPusherBase like IPacketPusher
{
    parameters:
        object mapping @mutable = default({}); // Map from input stream names to array of output stream names, empty string means no stream
        @class(StreamSplitter);
        @display("i=block/fork");
}
