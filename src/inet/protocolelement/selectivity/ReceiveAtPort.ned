//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.common.SimpleModule;

simple ReceiveAtPort extends SimpleModule
{
    parameters:
        @class(ReceiveAtPort);
        int port;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
