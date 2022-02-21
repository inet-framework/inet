//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

namespace inet;

//
// This tag can be attached to individual packets the specify the data rate and
// packet rate. The values are meaningful for a stream of packets which this
// packet is part of.
//
class RateTag extends TagBase
{
    bps datarate;
    double packetrate;
}
