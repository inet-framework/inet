//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.cutthrough;

import inet.protocolelement.common.PacketStreamer;

//
// Receives packet streams from a CutthroughSource and converts them back into
// complete packets, serving as the endpoint for cut-through switching paths.
//
// Operation:
// - Receives packet streaming events from a CutthroughSource
// - Reconstructs complete packets from the stream fragments
// - Forwards reconstructed packets to upper layers
// - Maintains proper timing of packet reception
// - Completes the cut-through switching path
//
// @see ~CutthroughSource, ~PacketStreamer
//
simple CutthroughSink extends PacketStreamer
{
    parameters:
        @class(CutthroughSink);
        @display("i=block/routing");
}
