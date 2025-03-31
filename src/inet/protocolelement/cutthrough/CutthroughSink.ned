//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.cutthrough;

import inet.protocolelement.common.PacketStreamer;

//
// Receives and processes packets that have been transmitted using cutthrough
// switching. This module extracts the original packet data from the streamed
// packet, removing cutthrough-specific tags and buffers. It works in conjunction
// with CutthroughSource to implement the cutthrough switching mechanism, which
// allows packet forwarding to begin before the entire packet is received,
// thereby reducing latency in network switches.
//
simple CutthroughSink extends PacketStreamer
{
    parameters:
        @class(CutthroughSink);
        @display("i=block/routing");
}
