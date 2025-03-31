//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.protocolelement.common.PacketStreamer;

//
// Completes the cutthrough switching process for Ethernet frames by handling
// packets marked with cutthrough tags. Processes streamed packets by replacing
// stream buffer chunks with actual data at the end of streaming. Works in
// conjunction with EthernetCutthroughSource to implement low-latency packet
// forwarding, reducing switching delays compared to traditional store-and-forward
// switching.
//
simple EthernetCutthroughSink extends PacketStreamer
{
    parameters:
        @class(EthernetCutthroughSink);
        @display("i=block/routing");
}
