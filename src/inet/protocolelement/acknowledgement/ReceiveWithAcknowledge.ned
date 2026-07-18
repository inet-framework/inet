//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.acknowledgement;

import inet.queueing.base.PacketPusherBase;

simple ReceiveWithAcknowledge extends PacketPusherBase
{
    parameters:
        @class(ReceiveWithAcknowledge);
        @display("i=block/rxtx");
    gates:
        output ackOut;
}
