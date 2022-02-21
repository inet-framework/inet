//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;

namespace inet;

//
// Determines the multicast loop mode
//
class MulticastReq extends TagBase
{
    bool multicastLoop = false;   // if true, sent multicast datagrams will be looped back
}
