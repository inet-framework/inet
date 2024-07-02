//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

namespace inet;

class ProgressTag extends TagBase
{
    bps datarate;
    b position;
    b extraProcessableLength;
    simtime_t extraProcessableDuration;
}
