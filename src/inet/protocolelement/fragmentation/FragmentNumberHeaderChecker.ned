//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.fragmentation;

import inet.queueing.base.PacketFilterBase;

//
// Verifies fragment number headers in incoming packets. Checks the validity
// of fragment sequence numbers and other header fields to ensure proper
// fragment processing.
//
simple FragmentNumberHeaderChecker extends PacketFilterBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(FragmentNumberHeaderChecker);
        @display("i=block/checker");
}
