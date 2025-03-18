//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.authentication;

import inet.protocolelement.contract.IProtocolHeaderInserter;
import inet.queueing.base.PacketFlowBase;

//
// Inserts a Message Authentication Code (MAC) header at the front of packets to
// provide authentication in network protocols. Adds a configurable-length header
// (default 4 bytes) represented by a ByteCountChunk. Works in conjunction with
// MessageAuthenticationCodeChecker which verifies and removes the MAC header.
// Implements a simplified authentication mechanism without actual cryptographic
// operations, serving as a placeholder for more complex security implementations
// in network simulations.
//
simple MessageAuthenticationCodeInserter extends PacketFlowBase like IProtocolHeaderInserter
{
    parameters:
        int headerLength @unit(b) = default(4B);
        @class(MessageAuthenticationCodeInserter);
        @display("i=block/inserter");
}
