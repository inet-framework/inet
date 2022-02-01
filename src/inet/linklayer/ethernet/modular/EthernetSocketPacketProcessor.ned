//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.queueing.base.PacketPusherBase;
import inet.queueing.contract.IPacketFlow;

simple EthernetSocketPacketProcessor extends PacketPusherBase like IPacketFlow
{
    parameters:
        @class(EthernetSocketPacketProcessor);
        @display("i=block/cogwheel");
        string socketTableModule;
}

