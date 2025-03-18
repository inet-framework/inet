//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.ordering;

import inet.queueing.base.PacketPusherBase;

//
// Reorders out-of-sequence packets based on their sequence numbers. Buffers
// packets until they can be delivered in the correct order, ensuring sequential
// processing of packets despite network reordering.
//
simple Reordering extends PacketPusherBase
{
    parameters:
        @class(Reordering);
        @display("i=block/rx");
}
