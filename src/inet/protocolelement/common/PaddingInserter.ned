//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Adds padding to packets to ensure they meet a minimum length requirement or
// to round up to a specific length. This module calculates the required padding
// length based on the packet's current length and the configured parameters. If
// padding is needed, it adds either a ByteCountChunk (for byte-aligned padding)
// or a BitCountChunk (for bit-level padding) at the specified position (front
// or back of the packet). This is commonly used in protocols that require
// packets to have a minimum size or to align to specific boundaries.
//
simple PaddingInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        int minLength @unit(b);
        int roundingLength @unit(b) = default(1B);
        string insertionPosition @enum("front","back") = default("back");
        @class(PaddingInserter);
        @display("i=block/inserter");
}
