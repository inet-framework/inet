//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.acknowledgement;

//
// Receives packets with sequence numbers and generates acknowledgment packets
// in response. Implements the receiver side of a reliable communication protocol
// with positive acknowledgments.
//
// Operation:
// - Extracts sequence number from incoming data packets
// - Forwards data packets to upper layers
// - Creates acknowledgment packets with matching sequence numbers
// - Sends acknowledgments through dedicated gate
// - Registers with AccessoryProtocol service
//
// @see ~SendWithAcknowledge, ~Resending
//
simple ReceiveWithAcknowledge
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input in;
        output out;
        output ackOut;
}
