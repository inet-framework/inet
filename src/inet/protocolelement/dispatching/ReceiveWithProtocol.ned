//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.dispatching;

import inet.common.SimpleModule;

simple ReceiveWithProtocol extends SimpleModule
{
    parameters:
        @class(ReceiveWithProtocol);
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
