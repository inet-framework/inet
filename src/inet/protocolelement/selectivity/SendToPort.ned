//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

//
// Adds a destination port header to outgoing packets. Prepares packets
// for delivery to a specific transport layer port.
//
// Operation:
// - Creates a destination port header with the configured port number
// - Inserts the header at the front of outgoing packets
// - Registers with AccessoryProtocol service
// - Enables port-based multiplexing at the transport layer
//
// @see ~ReceiveAtPort, ~SendToL3Address, ~SendToMacAddress
//
simple SendToPort
{
    parameters:
        int port;
        @display("i=block/tx");
    gates:
        input in;
        output out;
}
