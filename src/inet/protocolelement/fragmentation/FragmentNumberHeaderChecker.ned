//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation;

import inet.queueing.base.PacketFilterBase;

//
// Validates fragment number headers in packets and filters out invalid fragments.
// Acts as a gatekeeper for the defragmentation process by ensuring only valid
// fragments are processed.
//
// Operation:
// - Examines packets for fragment number headers
// - Verifies header integrity and validity
// - Passes valid fragments to the defragmenter
// - Filters out packets with invalid or missing fragment headers
// - Processes headers at configurable position (front or back)
//
// @see ~FragmentNumberHeaderBasedDefragmenter, ~FragmentNumberHeaderInserter
//
simple FragmentNumberHeaderChecker extends PacketFilterBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(FragmentNumberHeaderChecker);
        @display("i=block/checker");
}
