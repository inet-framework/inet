//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet::bgp;

enum BgpSessionType
{
    IGP = 0;
    EGP = 1;
    INCOMPLETE = 2;
}

enum BgpPathSegmentType
{
    AS_SET = 1;
    AS_SEQUENCE = 2;
}
