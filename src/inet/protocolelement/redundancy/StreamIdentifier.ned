//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Identifies packet streams based on packet content and attaches stream request
// tags. Enables flexible stream-based processing by mapping packet characteristics
// to logical stream identifiers.
//
// Operation:
// - Examines packets using configurable packet filter expressions
// - Matches packet content against mapping rules
// - Attaches StreamReq tags to packets when matches are found
// - Optionally adds sequence numbering to streams
// - Provides the foundation for stream-based redundancy
//
// @see ~StreamDecoder, ~StreamIdentifierLayer
//
simple StreamIdentifier extends PacketFlowBase like IPacketFlow
{
    parameters:
        bool hasSequenceNumbering = default(true);
        object mapping @mutable = default([]); // Array of objects, where each object specifies a stream name and a packetFilter, for example [{stream: "s0", packetFilter: "*"}, ...]
        @class(StreamIdentifier);
        @display("i=block/blackboard");
}
