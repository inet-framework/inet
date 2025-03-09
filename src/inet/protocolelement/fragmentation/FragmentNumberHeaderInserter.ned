//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation;

import inet.queueing.base.PacketFlowBase;

//
// Inserts fragment number headers into packets to enable fragmentation and
// reassembly. Provides the header infrastructure needed for packet fragmentation
// without performing the actual fragmentation.
//
// Operation:
// - Adds FragmentNumberHeader to packets
// - Sets appropriate fragment flags and sequence numbers
// - Inserts headers at configurable position (front or back)
// - Prepares packets for potential fragmentation
// - Maintains packet flow while adding fragmentation capability
//
// Particularly useful for:
// - Preparing packets for fragmentation
// - Implementing protocol-specific fragmentation headers
// - Supporting fragmentation in protocol stacks
// - Testing fragmentation header processing
// - Building modular fragmentation systems
//
// @see ~FragmentNumberHeaderChecker, ~FragmentNumberHeaderBasedFragmenter
//
simple FragmentNumberHeaderInserter extends PacketFlowBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(FragmentNumberHeaderInserter);
        @display("i=block/inserter");
}
