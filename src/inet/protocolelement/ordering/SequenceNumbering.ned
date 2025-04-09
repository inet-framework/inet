//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.ordering;

import inet.queueing.base.PacketFlowBase;

//
// Adds sequence numbers to outgoing packets. Maintains an internal counter
// to assign unique, sequential numbers to packets, enabling downstream modules
// to detect duplicates and reorder packets.
//
// @see Reordering, DuplicateRemoval
//
simple SequenceNumbering extends PacketFlowBase
{
    parameters:
        @class(SequenceNumbering);
}
