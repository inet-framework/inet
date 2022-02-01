//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.base;

import inet.queueing.base.PacketPusherBase;

simple DefragmenterBase extends PacketPusherBase
{
    parameters:
        bool deleteSelf = default(false);
        @display("i=block/join");
        @class(DefragmenterBase);
}
