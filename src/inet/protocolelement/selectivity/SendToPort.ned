//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

simple SendToPort
{
    parameters:
        int port;
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
