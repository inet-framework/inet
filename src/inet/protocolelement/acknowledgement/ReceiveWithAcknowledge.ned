//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.acknowledgement;

import inet.common.SimpleModule;

//
// Receives packets with sequence numbers, processes them, and generates
// acknowledgement packets. Extracts the sequence number from each incoming packet,
// forwards the original data packet through the "out" gate, and sends an
// acknowledgement packet containing the same sequence number through the "ackOut"
// gate. Works in conjunction with SendWithAcknowledge to implement reliable
// communication with acknowledgements.
//
simple ReceiveWithAcknowledge extends SimpleModule
{
    parameters:
        @class(ReceiveWithAcknowledge);
        @display("i=block/rxtx");
    gates:
        input in;
        output out;
        output ackOut;
}
