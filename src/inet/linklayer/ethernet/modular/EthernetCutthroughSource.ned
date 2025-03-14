//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.common.PacketDestreamer;

//
// Initiates Ethernet cut-through switching by determining if a packet is eligible
// for cut-through and creating the necessary stream buffer. Computes the cut-through
// switching header size, which determines where in the packet the cut-through process
// begins. Creates a StreamBufferChunk for the cut-through data and adds a CutthroughTag
// to the packet. Essential component in reducing end-to-end delay by allowing transmission
// to begin before reception is complete.
//
// @see EthernetCutthroughBarrier, EthernetCutthroughSink, EthernetCutthroughLayer, EthernetCutthroughInterface
//
simple EthernetCutthroughSource extends PacketDestreamer
{
    parameters:
        string macTableModule;
        int cutthroughSwitchingHeaderSize @unit(b) = default(0b);
        @class(EthernetCutthroughSource);
        @display("i=block/routing");
}
