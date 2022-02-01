//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketFlow;

simple Ieee8021qSocketPacketProcessor extends PacketPusherBase like IPacketFlow
{
    parameters:
        @class(Ieee8021qSocketPacketProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}

