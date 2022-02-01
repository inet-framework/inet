//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.server;

import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;

//
// This module repeatedly pulls packets from the connected packet provider and
// pushes the packet into the connected packet consumer without delay.
//
simple InstantServer extends PacketServerBase like IPacketServer
{
    parameters:
        @class(InstantServer);
}
