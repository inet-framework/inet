//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.acknowledgement;

simple SendWithAcknowledge
{
    parameters:
        double acknowledgeTimeout @unit(s);
        @display("i=block/rxtx");
    gates:
        input in;
        input ackIn;
        output out;
}
