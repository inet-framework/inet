//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketPusher;

//
// This module duplicates incoming packets based on the stream they are part of.
// The stream is determined by the StreamReq tag that is attached to the packet.
// The number of outgoing packet is determined by the mapping parameter. Each
// outgoing packet will have an attached StreamReq with the tag name taken from
// the mapping parameter.
//
// @see ~StreamMerger
//
simple StreamSplitter extends PacketPusherBase like IPacketPusher
{
    parameters:
        object mapping @mutable = default({}); // map from input stream names to array of output stream names, empty string means no stream
        @class(StreamSplitter);
        @display("i=block/fork");
}
