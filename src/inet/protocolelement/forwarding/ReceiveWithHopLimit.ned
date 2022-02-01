//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.forwarding;

simple ReceiveWithHopLimit
{
    parameters:
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
