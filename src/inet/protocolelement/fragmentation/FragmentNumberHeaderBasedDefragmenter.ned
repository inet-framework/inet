//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation;

import inet.protocolelement.fragmentation.base.DefragmenterBase;
import inet.protocolelement.fragmentation.contract.IPacketDefragmenter;

//
// Reassembles original packets from fragments using fragment number headers
// for identification and ordering. Complements the FragmentNumberHeaderBasedFragmenter
// with a header-based defragmentation mechanism.
//
// Operation:
// - Receives packet fragments from lower layers
// - Identifies fragments using their FragmentNumberHeader information
// - Processes headers at configurable position (front or back)
// - Buffers fragments until all pieces of a packet are received
// - Reassembles original packets from collected fragments in correct order
//
// @see ~FragmentNumberHeaderBasedFragmenter, ~FragmentNumberHeaderChecker, ~DefragmenterBase
//
simple FragmentNumberHeaderBasedDefragmenter extends DefragmenterBase like IPacketDefragmenter
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(FragmentNumberHeaderBasedDefragmenter);
}
