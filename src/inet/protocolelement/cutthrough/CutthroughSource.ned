//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.cutthrough;

import inet.protocolelement.common.PacketDestreamer;

//
// Initiates cut-through switching by beginning packet forwarding before the
// complete packet has been received, significantly reducing forwarding latency.
//
// Operation:
// - Begins forwarding packets after receiving a configurable number of bits
// - Streams packet fragments to a CutthroughSink at another port
// - Maintains proper timing of packet transmission
// - Forwards header information before the complete packet is received
// - Enables low-latency packet forwarding in network devices
//
// Particularly useful for:
// - Implementing cut-through switching in network devices
// - Reducing end-to-end latency in time-sensitive networks
// - Creating low-latency forwarding paths in switches
// - Supporting time-sensitive networking applications
// - Modeling specialized hardware switching architectures
//
// @see ~CutthroughSink, ~PacketDestreamer
//
simple CutthroughSource extends PacketDestreamer
{
    parameters:
        int cutthroughPosition @unit(b);
        @class(CutthroughSource);
        @display("i=block/routing");
}
