//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.ipv4modular;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple Ipv4SocketCommandProcessor extends PacketFlowBase like IPacketFlow
{
    parameters:
        @class(Ipv4SocketCommandProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
    gates:
        output socketOut;
}
