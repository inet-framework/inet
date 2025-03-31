//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.selectivity;

//
// Adds a destination network layer address header to packets. The header
// contains the configured destination address and is used by the corresponding
// ReceiveAtL3Address module to filter packets.
//
simple SendToL3Address
{
    parameters:
        string address;
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
