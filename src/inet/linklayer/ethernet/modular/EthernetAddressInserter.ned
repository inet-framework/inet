//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple EthernetAddressInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        string interfaceTableModule;
        @class(EthernetAddressInserter);
        @display("i=block/inserter");
}
