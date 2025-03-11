//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

//
// Adds a destination network layer (L3) address header to outgoing packets.
// Prepares packets for delivery to a specific IPv4 address.
//
// Operation:
// - Creates a destination L3 address header with the configured address
// - Inserts the header at the front of outgoing packets
// - Adds appropriate protocol tags for packet processing
// - Registers with AccessoryProtocol service
// - Enables forwarding to the specified destination
//
// Particularly useful for:
// - Implementing network layer addressing
// - Creating protocol stacks with address-based routing
// - Building virtual network interfaces with specific addressing
// - Supporting IP-based packet forwarding
// - Implementing basic network layer routing functionality
//
// @see ~ReceiveAtL3Address, ~SendToMacAddress, ~SendToPort
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
