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
// Reassembles original packets from fragments based on fragment number headers.
// Collects fragments and reconstructs the complete packet when all fragments
// have been received, using sequence numbers to ensure correct ordering.
//
simple FragmentNumberHeaderBasedDefragmenter extends DefragmenterBase like IPacketDefragmenter
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(FragmentNumberHeaderBasedDefragmenter);
}
