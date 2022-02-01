//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.ordering;

import inet.queueing.base.PacketPusherBase;

simple DuplicateRemoval extends PacketPusherBase
{
    parameters:
        @class(DuplicateRemoval);
        @display("i=block/rx");
}
