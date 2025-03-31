//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.acknowledgement;

//
// Receives packets with sequence numbers, processes them, and generates
// acknowledgement packets. Extracts the sequence number from each incoming packet,
// forwards the original data packet through the "out" gate, and sends an
// acknowledgement packet containing the same sequence number through the "ackOut"
// gate. Works in conjunction with SendWithAcknowledge to implement reliable
// communication with acknowledgements.
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
