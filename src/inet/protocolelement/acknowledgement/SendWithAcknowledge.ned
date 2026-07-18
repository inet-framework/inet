//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.acknowledgement;

import inet.queueing.base.PacketFlowBase;

simple SendWithAcknowledge extends PacketFlowBase
{
    parameters:
        @class(SendWithAcknowledge);
        double acknowledgeTimeout @unit(s);
        @display("i=block/rxtx");
    gates:
        input ackIn;
}
