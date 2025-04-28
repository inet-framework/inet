//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.acknowledgement;

import inet.common.SimpleModule;

simple ReceiveWithAcknowledge extends SimpleModule
{
    parameters:
        @class(ReceiveWithAcknowledge);
        @display("i=block/rxtx");
    gates:
        input in;
        output out;
        output ackOut;
}
