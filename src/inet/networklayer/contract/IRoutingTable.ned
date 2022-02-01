//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.contract;

//
// This module interface provides an abstraction for different routing tables.
//
moduleinterface IRoutingTable
{
    parameters:
        @display("i=block/table");
        bool forwarding;
        bool multicastForwarding;
}

