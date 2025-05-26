//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.common.SimpleModule;

simple SendToL3Address extends SimpleModule
{
    parameters:
        @class(SendToL3Address);
        string address;
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
