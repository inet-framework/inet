//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.common.SimpleModule;

simple QosAckHandler extends SimpleModule
{
    parameters:
        @class(QosAckHandler);
        @display("i=block/table2");
}

