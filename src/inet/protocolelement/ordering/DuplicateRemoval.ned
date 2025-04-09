//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.ordering;

import inet.queueing.base.PacketPusherBase;

//
// Filters out duplicate packets based on sequence numbers. Keeps track of the
// last processed sequence number and discards packets with the same sequence
// number to prevent duplicate packet processing. Note that in order for this
// strategy to work, packets must arrive in the correct order.
//
simple DuplicateRemoval extends PacketPusherBase
{
    parameters:
        @class(DuplicateRemoval);
        @display("i=block/rx");
}
