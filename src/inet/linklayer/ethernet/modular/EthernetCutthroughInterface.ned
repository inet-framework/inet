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
// This module represents an Ethernet network interface with Ethernet cut-through
// switching support. In contrast with store-and-forward switching, Ethernet
// cut-through switching can begin the transmission of the outgoing packet before
// the reception of the corresponding incoming packet ends. Ethernet cut-through
// switching can significantly reduce end-to-end delay in the network.
//
// There are some important limitations with the current Ethernet cut-through
// switching implementation:
//  - FCS is not checked for cut-through packets in Ethernet switches
//  - packet data cannot be changed for cut-through packets in Ethernet switches
//  - Ethernet frame preemption is not supported with cut-through packets
//
// The Ethernet FCS is simply forwarded as it is for cut-through packets. Moreover,
// the packet data cannot be changed inside the Ethernet switch, because it would
// imply changing the FCS of the packet. Changing the FCS of the packet would
// require checking the current FCS and aborting incorrect frames. The Ethernet
// cut-through implementation also assumes that transmissions are not updated on
// the channel as is the case for Ethernet frame preemption. The packet contents
// at the start of the reception is the same as the packet contents at the end of
// the reception.
//
// The main reason for these limitations is that with Ethernet cut-through
// switching a received packet can reach far away both inside the network node
// and also in the network and the current architecture doesn't support updating
// this process.
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

