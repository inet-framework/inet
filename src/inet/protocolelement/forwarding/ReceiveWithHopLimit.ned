//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.forwarding;

//
// Processes incoming packets with hop limit headers, verifying that packets
// have not exceeded their maximum allowed hop count. Extracts hop limit
// information and adds it as a tag for upper protocol layers.
//
// Operation:
// - Filters packets based on their hop limit value
// - Drops packets with zero hop limit
// - Extracts the hop limit value from packet headers
// - Adds hop limit indication tags for upper layer protocols
// - Maintains protocol dispatch information
//
// @see ~SendWithHopLimit, ~Forwarding
//
simple ReceiveWithHopLimit
{
    parameters:
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
