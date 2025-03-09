//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

//
// Filters packets based on their destination port number. Only forwards
// packets that are addressed to the configured port number.
//
// Operation:
// - Examines incoming packets for destination port headers
// - Compares the destination port with the configured port
// - Passes packets with matching destination port
// - Removes the port header from matching packets
// - Registers with AccessoryProtocol service
//
// @see ~SendToPort, ~ReceiveAtL3Address, ~ReceiveAtMacAddress
//
simple ReceiveAtPort
{
    parameters:
        int port;
        @display("i=block/rx");
    gates:
        input in;
        output out;
}
