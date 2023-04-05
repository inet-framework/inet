//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.linklayer.ethernet.LayeredEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetMacLayer;
import inet.physicallayer.wired.ethernet.IEthernetPhyLayer;

//
// This module represents an Ethernet network interface with cut-through support.
//
// There are a few important limitations with the current Ethernet cut-through
// implementation. Ethernet cut-through and Ethernet preemption cannot be used
// simulatenously. Also, Ethernet frame corruption is not supported with cut-through.
//
// The Ethernet cut-through implementation assumes that transmissions are not
// updated (e.g. preemption). The packet contents at the start of the reception
// is the same as the packet contents at the end of the reception. The reason
// is that with cut-through a received packet can reach far away in the network
// before a preemption happens.
//
// Similarly, the Ethernet FCS is always assumed to be correct for cut-through
// packets. By the time the Ethernet FCS is received by a switch the cut-through
// packet can reach far away in the network, and it's currently not possible to
// abort the cut-through transmission.
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

