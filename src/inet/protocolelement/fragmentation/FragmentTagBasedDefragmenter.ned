//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation;

import inet.protocolelement.fragmentation.base.DefragmenterBase;
import inet.protocolelement.fragmentation.contract.IPacketDefragmenter;

//
// Reassembles original packets from fragments using packet tags to identify
// and track fragment relationships. Complements the FragmentTagBasedFragmenter
// with a lightweight defragmentation mechanism.
//
// Operation:
// - Receives packet fragments from lower layers
// - Identifies fragments using their attached FragmentTag
// - Buffers fragments until all pieces of a packet are received
// - Reassembles original packets from collected fragments
// - Forwards complete packets to upper layers
//
// Particularly useful for:
// - Implementing defragmentation in simulation-only scenarios
// - Testing defragmentation without protocol header overhead
// - Creating simplified defragmentation mechanisms
// - Studying defragmentation effects without protocol specifics
// - Implementing custom defragmentation schemes
//
// @see ~FragmentTagBasedFragmenter, ~DefragmenterBase
//
simple FragmentTagBasedDefragmenter extends DefragmenterBase like IPacketDefragmenter
{
    parameters:
        @class(FragmentTagBasedDefragmenter);
}
