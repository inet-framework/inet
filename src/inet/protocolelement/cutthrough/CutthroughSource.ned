//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.cutthrough;

import inet.protocolelement.common.PacketDestreamer;

//
// Initiates cutthrough switching for packets, allowing transmission to begin
// before the entire packet is received. This module specifies the position in
// the packet where cutthrough switching can begin, enabling low-latency packet
// forwarding in network devices. It works in conjunction with CutthroughSink to
// implement the cutthrough switching mechanism, which significantly reduces
// forwarding latency compared to traditional store-and-forward switching.
//
simple CutthroughSource extends PacketDestreamer
{
    parameters:
        int cutthroughPosition @unit(b);
        @class(CutthroughSource);
        @display("i=block/routing");
}
