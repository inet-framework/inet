//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

//
// Filters packets based on their destination MAC address. Only forwards
// packets that are addressed to the configured MAC address and sets this
// address as the MAC address of the containing network interface.
//
// Operation:
// - Examines incoming packets for destination MAC address headers
// - Compares the destination address with the configured address
// - Passes packets with matching destination address
// - Removes the MAC address header from matching packets
// - Sets the MAC address of the containing network interface
//
// @see ~SendToMacAddress, ~ReceiveAtL3Address, ~ReceiveAtPort
//
simple ReceiveAtMacAddress
{
    parameters:
        string address;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
