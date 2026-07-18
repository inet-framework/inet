//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.dispatching;

import inet.queueing.base.PacketFlowBase;

simple SendWithProtocol extends PacketFlowBase
{
    parameters:
        @class(SendWithProtocol);
        @display("i=block/tx");
}
