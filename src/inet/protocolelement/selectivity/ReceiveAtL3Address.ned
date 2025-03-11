//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

//
// Filters packets based on their destination network layer (L3) address.
// Only forwards packets that are addressed to the configured IPv4 address.
//
// Operation:
// - Examines incoming packets for destination L3 address headers
// - Compares the destination address with the configured address
// - Passes packets with matching destination address
// - Removes the L3 address header from matching packets
// - Registers with AccessoryProtocol service
//
// Particularly useful for:
// - Implementing network layer address filtering
// - Creating protocol stacks with address-based selectivity
// - Building virtual network interfaces with specific addresses
// - Supporting IP-based packet filtering
// - Implementing basic network layer routing functionality
//
// @see ~SendToL3Address, ~ReceiveAtMacAddress, ~ReceiveAtPort
//
simple ReceiveAtL3Address
{
    parameters:
        string address;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
