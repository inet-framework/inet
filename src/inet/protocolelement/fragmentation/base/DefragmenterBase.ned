//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation.base;

import inet.queueing.base.PacketPusherBase;

//
// Base type for packet defragmenters that reassemble original packets from
// fragments. Collects and stores fragments until all fragments of a packet are
// received, then reconstructs the original packet.
//
simple DefragmenterBase extends PacketPusherBase
{
    parameters:
        bool deleteSelf = default(false);
        @display("i=block/join");
        @class(DefragmenterBase);
}
