//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.dispatching;

import inet.common.SimpleModule;

simple SendWithProtocol extends SimpleModule
{
    parameters:
        @class(SendWithProtocol);
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
