//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.fragmentation;

import inet.queueing.base.PacketFlowBase;

//
// Inserts fragment number headers into packets. The headers contain sequence
// information that allows fragments to be properly ordered and reassembled
// at the destination.
//
simple FragmentNumberHeaderInserter extends PacketFlowBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(FragmentNumberHeaderInserter);
        @display("i=block/inserter");
}
