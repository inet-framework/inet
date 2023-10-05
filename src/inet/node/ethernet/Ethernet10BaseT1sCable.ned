//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.node.ethernet;

import ned.DatarateChannel;

channel Ethernet10BaseT1sCable extends DatarateChannel
{
    double length @unit(m);
    delay = default(replaceUnit(length / 2e8, "s"));
    datarate = default(10Mbps);
}
