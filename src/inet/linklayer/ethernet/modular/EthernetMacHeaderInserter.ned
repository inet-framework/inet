//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.protocolelement.contract.IProtocolHeaderInserter;
import inet.queueing.base.PacketFlowBase;

simple EthernetMacHeaderInserter extends PacketFlowBase like IProtocolHeaderInserter
{
    parameters:
        string interfaceTableModule;
        @class(EthernetMacHeaderInserter);
        @display("i=block/inserter");
}
