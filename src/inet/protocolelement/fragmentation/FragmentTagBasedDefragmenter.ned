//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.fragmentation;

import inet.protocolelement.fragmentation.base.DefragmenterBase;
import inet.protocolelement.fragmentation.contract.IPacketDefragmenter;

//
// Reassembles original packets from fragments based on fragment tags. Uses
// tag information attached to packets to identify fragments belonging to the
// same original packet and reconstruct it in the correct order.
//
simple FragmentTagBasedDefragmenter extends DefragmenterBase like IPacketDefragmenter
{
    parameters:
        @class(FragmentTagBasedDefragmenter);
}
