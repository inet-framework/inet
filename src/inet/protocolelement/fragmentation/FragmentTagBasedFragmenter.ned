//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation;

import inet.protocolelement.fragmentation.base.FragmenterBase;
import inet.protocolelement.fragmentation.contract.IPacketFragmenter;

module FragmentTagBasedFragmenter extends FragmenterBase like IPacketFragmenter
{
    parameters:
        @class(FragmentTagBasedFragmenter);
}
