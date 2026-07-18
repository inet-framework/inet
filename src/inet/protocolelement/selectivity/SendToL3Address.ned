//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.queueing.base.PacketFlowBase;

simple SendToL3Address extends PacketFlowBase
{
    parameters:
        @class(SendToL3Address);
        string address;
        @display("i=block/tx");
}
