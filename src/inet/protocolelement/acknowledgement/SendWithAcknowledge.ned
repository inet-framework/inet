//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.acknowledgement;

//
// Adds sequence numbers to outgoing packets and processes incoming acknowledgments.
// Implements the sender side of a reliable communication protocol with positive
// acknowledgments and timeout-based retransmission.
//
// Operation:
// - Assigns sequence numbers to outgoing data packets
// - Maintains timers for unacknowledged packets
// - Processes incoming acknowledgment packets
// - Retransmits packets if acknowledgment timeout occurs
// - Registers with AccessoryProtocol service
//
// Particularly useful for:
// - Implementing reliable communication protocols
// - Creating timeout-based retransmission schemes
// - Building custom ARQ (Automatic Repeat Request) mechanisms
// - Testing network reliability and congestion control
// - Simulating protocols with positive acknowledgment
//
// @see ~ReceiveWithAcknowledge, ~Resending
//
simple SendWithAcknowledge
{
    parameters:
        double acknowledgeTimeout @unit(s);
        @display("i=block/rxtx");
    gates:
        input in;
        input ackIn;
        output out;
}
