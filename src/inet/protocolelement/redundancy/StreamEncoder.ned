//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// This module encodes the stream name of a packet into several request tags
// attached to the packet. It looks up the stream in the mapping and attaches
// the necessary request tags to the packet based on the specified parameters.
//
// @see ~StreamDecoder
//
simple StreamEncoder extends PacketFlowBase like IPacketFlow
{
    parameters:
        object mapping @mutable = default([]); // array of objects, where each object maps a stream name to a VLAN id, and PCP value, for example [{stream: "s1", vlan: 12, pcp: 7}, ...]
        @class(StreamEncoder);
        @display("i=block/blackboard");
}
