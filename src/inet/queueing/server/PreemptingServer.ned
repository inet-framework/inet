//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.server;

import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;

simple PreemptingServer extends PacketServerBase like IPacketServer
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        double datarate @unit(bps);
        @class(PreemptingServer);
}
