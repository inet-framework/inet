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
// Adds standard Ethernet PHY headers to packets. Inserts a physical layer header
// consisting of a preamble (0x55) and Start Frame Delimiter (SFD) as defined in
// IEEE 802.3. Used in the Ethernet physical layer to prepare packets for
// transmission over the physical medium.
//
simple EthernetPhyHeaderInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(EthernetPhyHeaderInserter);
        @display("i=block/inserter");
}
