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
// Fragments packets into smaller pieces and attaches fragment tags to each
// fragment. These tags contain information that allows the fragments to be
// properly reassembled at the destination.
//
module FragmentTagBasedFragmenter extends FragmenterBase like IPacketFragmenter
{
    parameters:
        @class(FragmentTagBasedFragmenter);
}
