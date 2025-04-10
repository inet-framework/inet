//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mib;

import inet.common.SimpleModule;

simple Ieee80211Mib extends SimpleModule
{
    parameters:
        @class(Ieee80211Mib);
        @display("i=block/table");
}

