//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple EthernetFragmentPhyHeaderInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(EthernetFragmentPhyHeaderInserter);
        @display("i=block/inserter");
}
