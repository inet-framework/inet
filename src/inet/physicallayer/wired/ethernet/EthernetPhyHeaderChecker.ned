//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wired.ethernet;

import inet.queueing.base.PacketFilterBase;
import inet.queueing.contract.IPacketFilter;

//
// Validates and removes Ethernet PHY headers from incoming packets. Checks if
// the headers are correct and properly represented, dropping packets with invalid
// headers. Updates protocol tags to indicate the packet now contains Ethernet MAC
// data and should be dispatched to the MAC layer.
//
simple EthernetPhyHeaderChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        @class(EthernetPhyHeaderChecker);
        @display("i=block/checker");
}
