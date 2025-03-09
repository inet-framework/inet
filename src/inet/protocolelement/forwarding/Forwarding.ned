//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.forwarding;

//
// Implements basic packet forwarding functionality for network protocols.
// Determines the next hop for packets based on their destination address
// and adds appropriate routing information.
//
// Operation:
// - Examines the destination address in packet headers
// - Determines the next hop using a routing algorithm
// - Adds interface and MAC address tags for outgoing packets
// - Handles both direct delivery and forwarding cases
// - Maintains protocol dispatch information for protocol stack
//
// Particularly useful for:
// - Implementing routing functionality in network protocols
// - Building custom forwarding mechanisms
// - Creating simplified routing demonstrations
// - Testing network topologies with multiple hops
// - Implementing protocol-specific forwarding behavior
//
// @see ~SendWithHopLimit, ~ReceiveWithHopLimit
//
simple Forwarding
{
    parameters:
        string address = default("");
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
