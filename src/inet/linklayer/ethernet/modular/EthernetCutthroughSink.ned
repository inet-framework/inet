//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.common.PacketStreamer;

//
// Completes the Ethernet cut-through switching process at the end of streaming by
// replacing the StreamBufferChunk with the actual data. Detects cut-through packets
// by checking for the presence of a CutthroughTag, which is removed once the process
// is complete. Works in conjunction with other cut-through components to reduce
// end-to-end delay by allowing transmission to begin before reception is complete.
//
// @see EthernetCutthroughBarrier, EthernetCutthroughSource, EthernetCutthroughLayer, EthernetCutthroughInterface
//
simple EthernetCutthroughSink extends PacketStreamer
{
    parameters:
        @class(EthernetCutthroughSink);
        @display("i=block/routing");
}
