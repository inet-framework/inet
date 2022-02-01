//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple Ieee8022LlcSocketCommandProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(Ieee8022LlcSocketCommandProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}
