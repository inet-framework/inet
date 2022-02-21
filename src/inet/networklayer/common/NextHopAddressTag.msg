//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.networklayer.common.L3Address;

namespace inet;

class NextHopAddressReq extends TagBase
{
    L3Address nextHopAddress;      // may be unspecified
}
