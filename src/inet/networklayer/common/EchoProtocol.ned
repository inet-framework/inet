//
// Copyright (C) 2004, 2009 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.common;

import inet.common.SimpleModule;

//
// Provides a mechanism to test network layer connectivity
// using echo request/response messages similar to those of ICMP.
//
simple EchoProtocol extends SimpleModule
{
    parameters:
        @class(EchoProtocol);
        @display("i=block/control");
    gates:
        input ipIn;
        output ipOut;
}

