//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.acknowledgement;

//
// Sends packets with sequence numbers and waits for acknowledgements. Adds a
// sequence number header to outgoing packets and sets up a timer for each packet.
// If an acknowledgement is received before the timer expires, the packet is
// considered successfully delivered. If the timer expires (no acknowledgement
// received), it notifies the producer that the packet was not processed. Works in
// conjunction with ReceiveWithAcknowledge to implement reliable communication with
// acknowledgements.
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
