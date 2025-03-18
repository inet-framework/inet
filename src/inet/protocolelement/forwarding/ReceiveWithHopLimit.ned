//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.forwarding;

//
// Processes incoming packets with hop limit headers. Filters packets based on
// their hop limit value, discarding packets with expired hop limits. Extracts
// the hop limit information and attaches it as a tag to the packet.
//
simple ReceiveWithHopLimit
{
    parameters:
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
