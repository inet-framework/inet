//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.queueing.server;

import inet.queueing.base.PacketServerBase;
import inet.queueing.contract.IPacketServer;

//
// Implements a server that can be preempted during packet processing. Supports
// streaming mode where packet transmission can be interrupted and resumed.
// Processes packets at a configurable datarate and handles clock synchronization
// for accurate timing.
//
simple PreemptingServer extends PacketServerBase like IPacketServer
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double datarate @unit(bps);
        @class(PreemptingServer);
}
