//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.acknowledgement;

import inet.queueing.base.PacketPusherBase;

//
// Handles the retransmission of packets that were not acknowledged. Maintains a
// configurable number of retries for each packet, resending them when necessary.
// Works as part of the reliable communication mechanism together with
// SendWithAcknowledge and ReceiveWithAcknowledge to ensure packet delivery even
// in the presence of packet loss.
//
simple Resending extends PacketPusherBase
{
    parameters:
        int numRetries;
        @class(Resending);
        @display("i=block/tx");
}
