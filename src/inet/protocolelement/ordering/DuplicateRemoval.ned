//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.ordering;

import inet.queueing.base.PacketPusherBase;

//
// Filters out duplicate packets based on sequence numbers, ensuring each packet
// is processed only once. Works in conjunction with the SequenceNumbering module
// to provide reliable packet delivery.
//
// Operation:
// - Extracts sequence number from incoming packets
// - Compares with the last seen sequence number
// - Forwards packets with new sequence numbers
// - Discards packets with duplicate sequence numbers
// - Removes sequence number headers from forwarded packets
//
// @see ~SequenceNumbering, ~Reordering
//
simple DuplicateRemoval extends PacketPusherBase
{
    parameters:
        @class(DuplicateRemoval);
        @display("i=block/rx");
}
