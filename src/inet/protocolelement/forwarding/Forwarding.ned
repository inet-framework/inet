//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.forwarding;

import inet.common.SimpleModule;

simple Forwarding extends SimpleModule
{
    parameters:
        @class(Forwarding);
        string address = default("");
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
