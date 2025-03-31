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
// Validates and removes Ethernet fragment PHY headers from incoming packet
// fragments. Used in frame preemption (IEEE 802.1Qbu) to process fragments of
// preempted frames. Checks if the fragment headers are correct and properly
// represented, dropping fragments with invalid headers.
//
simple EthernetFragmentPhyHeaderChecker extends PacketFilterBase like IPacketFilter
{
    parameters:
        @class(EthernetFragmentPhyHeaderChecker);
        @display("i=block/checker");
}
