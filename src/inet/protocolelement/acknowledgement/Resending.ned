//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.acknowledgement;

import inet.queueing.base.PacketPusherBase;

simple Resending extends PacketPusherBase
{
    parameters:
        int numRetries;
        @class(Resending);
        @display("i=block/tx");
}
