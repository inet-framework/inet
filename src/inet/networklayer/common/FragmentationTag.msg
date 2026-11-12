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
    int mtu = -1;                // the largest packet that a fragment may be, in bytes, if smaller than the MTU of the outgoing interface; -1 means none (used by Ipv6)
}
