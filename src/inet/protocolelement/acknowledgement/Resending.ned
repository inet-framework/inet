//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.acknowledgement;

import inet.queueing.base.PacketPusherBase;

//
// Implements packet retransmission with a configurable number of retry attempts.
// Provides a simple mechanism for reliable communication by resending packets
// until successful delivery or maximum retries are reached.
//
// Operation:
// - Forwards packets to the next module
// - Tracks packet delivery success/failure
// - Retransmits packets when delivery fails
// - Limits retransmissions to configured maximum attempts
// - Notifies producer about final delivery status
//
// Particularly useful for:
// - Implementing basic reliability mechanisms
// - Creating simple ARQ (Automatic Repeat Request) schemes
// - Building custom error recovery protocols
// - Testing network resilience with controlled retransmissions
// - Simulating protocols with limited retry attempts
//
// @see ~SendWithAcknowledge, ~ReceiveWithAcknowledge
//
simple Resending extends PacketPusherBase
{
    parameters:
        int numRetries;
        @class(Resending);
        @display("i=block/tx");
}
