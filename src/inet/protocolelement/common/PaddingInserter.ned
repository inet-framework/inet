//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketFlowBase;
import inet.queueing.contract.IPacketFlow;

simple PaddingInserter extends PacketFlowBase like IPacketFlow
{
    parameters:
        int minLength @unit(b);
        int roundingLength @unit(b) = default(1B);
        string insertionPosition @enum("front","back") = default("back");
        @class(PaddingInserter);
        @display("i=block/inserter");
}
