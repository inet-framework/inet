//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.forwarding;

import inet.common.SimpleModule;

simple ReceiveWithHopLimit extends SimpleModule
{
    parameters:
        @class(ReceiveWithHopLimit);
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
