//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// This module identifies the stream of a packet using packet filter expressions.
// It can check for arbitrary packet data and assign and if a match was found then
// the stream is attached to the packet in a ~StreamReq tag.
//
simple StreamIdentifier extends PacketFlowBase like IPacketFlow
{
    parameters:
        bool hasSequenceNumbering = default(true);
        object mapping @mutable = default([]); // array of objects, where each object specifies a stream name and a packetFilter, for example [{stream: "s0", packetFilter: "*"}, ...]
        @class(StreamIdentifier);
        @display("i=block/blackboard");
}
