//
// Copyright (C) 2004, 2009 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.common;

//
// This module provides a mechanism to test network layer connectivity
// using echo request/response messages similar to that of ICMP.
//
simple EchoProtocol
{
    parameters:
        @display("i=block/control");
    gates:
        input ipIn;
        output ipOut;
}

