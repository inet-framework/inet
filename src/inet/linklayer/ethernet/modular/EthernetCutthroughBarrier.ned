//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketDelayerBase;
import inet.queueing.contract.IPacketDelayer;

//
// Acts as a barrier for Ethernet cut-through switching by completing the packet data.
// Replaces the StreamBufferChunk with actual data for cut-through packets and computes
// appropriate delay based on the start time and data rate of the stream buffer.
// Essential component in Ethernet cut-through switching implementation, which allows
// transmission to begin before reception is complete, reducing end-to-end delay.
//
// @see EthernetCutthroughSource, EthernetCutthroughSink, EthernetCutthroughLayer, EthernetCutthroughInterface
//
simple EthernetCutthroughBarrier extends PacketDelayerBase like IPacketDelayer
{
    parameters:
        @class(EthernetCutthroughBarrier);
}
