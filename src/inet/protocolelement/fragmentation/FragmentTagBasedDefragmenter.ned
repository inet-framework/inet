//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.fragmentation;

import inet.protocolelement.fragmentation.base.DefragmenterBase;
import inet.protocolelement.fragmentation.contract.IPacketDefragmenter;

simple FragmentTagBasedDefragmenter extends DefragmenterBase like IPacketDefragmenter
{
    parameters:
        @class(FragmentTagBasedDefragmenter);
}
