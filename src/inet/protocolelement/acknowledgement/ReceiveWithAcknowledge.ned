//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.acknowledgement;

simple ReceiveWithAcknowledge
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input in;
        output out;
        output ackOut;
}
