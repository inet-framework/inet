//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.fragmentation;

import inet.protocolelement.fragmentation.base.FragmenterBase;
import inet.protocolelement.fragmentation.contract.IPacketFragmenter;

//
// Fragments packets into smaller pieces and adds fragment number headers to
// each fragment. The headers contain information about fragment sequence and
// position, enabling proper reassembly at the destination.
//
module FragmentNumberHeaderBasedFragmenter extends FragmenterBase like IPacketFragmenter
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(FragmentNumberHeaderBasedFragmenter);
}
