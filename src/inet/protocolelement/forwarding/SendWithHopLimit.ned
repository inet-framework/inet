//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.forwarding;

import inet.queueing.base.PacketFlowBase;

simple SendWithHopLimit extends PacketFlowBase
{
    parameters:
        @class(SendWithHopLimit);
        int hopLimit;
        @display("i=block/tx");
}
