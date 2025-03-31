//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.forwarding;

//
// Adds a hop limit header to outgoing packets. The hop limit value is
// configurable and determines how many network hops the packet can traverse
// before being discarded.
//
simple SendWithHopLimit
{
    parameters:
        int hopLimit;
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
