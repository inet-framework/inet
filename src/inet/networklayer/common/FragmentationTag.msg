//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// Determines the fragmentation
//
class FragmentationReq extends TagBase
{
    bool dontFragment = false;   // "don't fragment" bit
}
