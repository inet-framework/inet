//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.queueing.base.PacketFlowBase;

simple SendToMacAddress extends PacketFlowBase
{
    parameters:
        @class(SendToMacAddress);
        string address = default("");
        @display("i=block/tx");
}
