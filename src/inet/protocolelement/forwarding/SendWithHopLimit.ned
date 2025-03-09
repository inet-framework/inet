//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.forwarding;

//
// Adds hop limit headers to outgoing packets, enabling hop count-based
// forwarding and loop prevention in network protocols. Sets the initial
// hop limit value for packets entering the network.
//
// Operation:
// - Attaches a hop limit header to each packet
// - Sets the initial hop limit value from parameters
// - Removes any existing dispatch protocol requests
// - Adds appropriate protocol tags for packet processing
// - Prepares packets for forwarding through the network
//
// Particularly useful for:
// - Implementing TTL/hop limit functionality in network protocols
// - Limiting packet propagation distance in networks
// - Preventing routing loops in complex topologies
// - Building protocol stacks with hop count restrictions
// - Creating bounded-scope multicast mechanisms
//
// @see ~ReceiveWithHopLimit, ~Forwarding
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
