//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Inserts Ethernet MAC address fields into outgoing packets. Creates and adds
// the source and destination address header fields at the front of each packet.
// Uses the destination address from the packet's MacAddressReq tag. If the
// source address is unspecified in the tag, it uses the MAC address of the
// requested target network interface. Updates packet protocol tags to reflect
// the added header.
//
simple EthernetAddressInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        string interfaceTableModule;
        @class(EthernetAddressInserter);
        @display("i=block/inserter");
}
