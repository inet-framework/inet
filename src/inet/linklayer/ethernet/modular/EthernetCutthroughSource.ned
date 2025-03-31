//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ethernet.modular;

import inet.protocolelement.common.PacketDestreamer;

//
// Initiates the cutthrough switching process for Ethernet frames by analyzing
// incoming packets and determining if they are eligible for cutthrough
// processing. Creates and manages stream buffer chunks for cutthrough data,
// marks packets with cutthrough tags, and begins transmission before the entire
// packet is received. Works in conjunction with EthernetCutthroughBarrier and
// EthernetCutthroughSink to implement low-latency packet forwarding that reduces
// switching delays compared to traditional store-and-forward switching.
//
simple EthernetCutthroughSource extends PacketDestreamer
{
    parameters:
        string macTableModule;
        int cutthroughSwitchingHeaderSize @unit(b) = default(0b);
        @class(EthernetCutthroughSource);
        @display("i=block/routing");
}
