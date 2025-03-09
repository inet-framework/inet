//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

//
// Adds padding to packets to ensure they meet minimum length requirements
// or to round packet lengths to specific boundaries.
//
// Operation:
// - Examines packet length and calculates required padding
// - Adds padding bits or bytes to reach minimum length or round to specified boundary
// - Can insert padding at either front or back of packet
// - Uses bit or byte padding depending on alignment requirements
//
// @see ~PacketFlowBase, ~IPacketFlow
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
