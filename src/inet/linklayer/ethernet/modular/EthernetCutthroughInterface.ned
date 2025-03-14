//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

//
// Ethernet network interface with cut-through switching support that can begin
// transmission of outgoing packets before reception of corresponding incoming packets
// ends, significantly reducing end-to-end delay in the network.
//
// Important limitations of the current implementation include:
//  - FCS is not checked for cut-through packets in Ethernet switches
//  - Packet data cannot be changed for cut-through packets in Ethernet switches
//  - Ethernet frame preemption is not supported with cut-through packets
//
// The Ethernet FCS is simply forwarded as is for cut-through packets. Packet data
// cannot be changed inside the Ethernet switch because it would require changing
// the FCS and checking/aborting incorrect frames. The implementation assumes that
// transmissions are not updated on the channel (unlike Ethernet frame preemption).
// Packet contents at the start of reception must be the same as at the end.
//
// These limitations exist because with cut-through switching, a received packet
// can propagate far within the network node and network, and the current architecture
// doesn't support updating this process.
//
// @see EthernetCutthroughBarrier, EthernetCutthroughSource, EthernetCutthroughSink, EthernetCutthroughLayer
//
module EthernetCutthroughInterface extends LayeredEthernetInterface
{
    parameters:
        string macTableModule;
        *.bitrate = default(this.bitrate);
        *.macTableModule = default(this.macTableModule);
        phyLayer.typename = default("EthernetStreamThroughPhyLayer");
        phyLayer.cutthroughSource.typename = default("EthernetCutthroughSource");
        phyLayer.cutthroughSink.typename = default("EthernetCutthroughSink");
}
