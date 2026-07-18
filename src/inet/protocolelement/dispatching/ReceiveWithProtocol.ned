//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.dispatching;

import inet.queueing.base.PacketPusherBase;

simple ReceiveWithProtocol extends PacketPusherBase
{
    parameters:
        @class(ReceiveWithProtocol);
        @display("i=block/rx");
}
