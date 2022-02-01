//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.ordering;

import inet.queueing.base.PacketPusherBase;

simple Reordering extends PacketPusherBase
{
    parameters:
        @class(Reordering);
        @display("i=block/rx");
}
