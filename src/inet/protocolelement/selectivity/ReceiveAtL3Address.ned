//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.common.SimpleModule;

simple ReceiveAtL3Address extends SimpleModule
{
    parameters:
        @class(ReceiveAtL3Address);
        string address;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
