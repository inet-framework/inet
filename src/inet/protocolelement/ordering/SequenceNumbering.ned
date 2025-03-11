//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.ordering;

import inet.queueing.base.PacketFlowBase;

//
// Adds sequence numbers to packets to enable ordering and duplicate detection
// in networks where packets may be reordered or duplicated during transmission.
//
// Operation:
// - Maintains an internal sequence number counter
// - Inserts a SequenceNumberHeader at the front of each packet
// - Increments the sequence number for each processed packet
// - Adds appropriate protocol tags to the packet
// - Passes the modified packet to the next module
//
// Particularly useful for:
// - Implementing reliable transport protocols
// - Supporting packet reordering at the receiver
// - Enabling duplicate packet detection
// - Providing the foundation for acknowledgment-based protocols
// - Ensuring correct packet processing order in multi-path networks
//
// @see ~Reordering, ~DuplicateRemoval
//
simple SequenceNumbering extends PacketFlowBase
{
    parameters:
        @class(SequenceNumbering);
}
