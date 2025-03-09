//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.ordering;

import inet.queueing.base.PacketPusherBase;

//
// Restores the original packet order based on sequence numbers, ensuring
// packets are delivered in the correct sequence regardless of network delays
// or routing differences.
//
// Operation:
// - Extracts sequence number from incoming packets
// - Buffers out-of-order packets in a map indexed by sequence number
// - Forwards packets in sequence number order
// - Maintains an expected sequence number counter
// - Removes sequence number headers from forwarded packets
//
// @see ~SequenceNumbering, ~DuplicateRemoval
//
simple Reordering extends PacketPusherBase
{
    parameters:
        @class(Reordering);
        @display("i=block/rx");
}
