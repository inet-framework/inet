//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

simple ReceiveAtPort
{
    parameters:
        int port;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
