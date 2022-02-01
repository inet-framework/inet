//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple EthernetSocketCommandProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(EthernetSocketCommandProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}

