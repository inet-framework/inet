//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

//
// Adds a destination MAC address header to outgoing packets. Prepares packets
// for delivery to a specific MAC address, either configured directly or taken
// from packet tags.
//
// Operation:
// - Creates a destination MAC address header
// - Uses address from MacAddressReq tag if present, otherwise uses configured address
// - Inserts the header at the front of outgoing packets
// - Adds appropriate protocol tags for packet processing
// - Registers with AccessoryProtocol service
//
// Particularly useful for:
// - Implementing link layer addressing
// - Creating protocol stacks with MAC-based routing
// - Building virtual network interfaces with specific addressing
// - Supporting Ethernet-based packet forwarding
// - Implementing basic link layer switching functionality
//
// @see ~ReceiveAtMacAddress, ~SendToL3Address, ~SendToPort
//
simple SendToMacAddress
{
    parameters:
        string address = default("");
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
