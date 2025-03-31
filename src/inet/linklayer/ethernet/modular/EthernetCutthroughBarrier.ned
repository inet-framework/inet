//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketDelayerBase;
import inet.queueing.contract.IPacketDelayer;

//
// Acts as a processing point in the Ethernet cutthrough switching mechanism,
// completing the cutthrough process by replacing stream buffer chunks with
// actual data. This module detects packets marked with cutthrough tags,
// processes them by extracting the streamed data at the cutthrough position,
// and computes appropriate delays based on data rate. It works in conjunction
// with EthernetCutthroughSource and EthernetCutthroughSink to implement
// low-latency packet forwarding that begins transmission before the entire
// packet is received.
//
simple EthernetCutthroughBarrier extends PacketDelayerBase like IPacketDelayer
{
    parameters:
        @class(EthernetCutthroughBarrier);
}
