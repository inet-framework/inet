//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple Ieee8021qSocketCommandProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(Ieee8021qSocketCommandProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}

