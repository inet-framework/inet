//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wired.ethernet;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Adds Ethernet fragment PHY headers to packet fragments for frame preemption
// (IEEE 802.1Qbu). Inserts different header types for first fragments (SMD-Sx)
// and continuation fragments (SMD-Cx), and maintains proper fragment and SMD
// numbering sequence. Works in conjunction with the fragmentation mechanism to
// support preemptable frame transmission.
//
simple EthernetFragmentPhyHeaderInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(EthernetFragmentPhyHeaderInserter);
        @display("i=block/inserter");
}
