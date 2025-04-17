//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8021q;

import inet.common.SimpleModule;

simple Ieee8021qSocketTable extends SimpleModule
{
    parameters:
        @class(Ieee8021qSocketTable);
        @display("i=block/table");
}

