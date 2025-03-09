//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.authentication;

import inet.protocolelement.contract.IProtocolHeaderInserter;
import inet.queueing.base.PacketFlowBase;

//
// Computes and inserts message authentication codes (MACs) into packet headers
// to provide data integrity and authenticity in network communications.
//
// Operation:
// - Calculates a MAC based on packet content and a shared secret
// - Inserts the MAC into the packet header
// - Preserves the original packet content
// - Enables verification at the receiving end
// - Provides protection against tampering and forgery
//
// @see ~MessageAuthenticationCodeChecker, ~PacketFlowBase
//
simple MessageAuthenticationCodeInserter extends PacketFlowBase like IProtocolHeaderInserter
{
    parameters:
        int headerLength @unit(b) = default(4B);
        @class(MessageAuthenticationCodeInserter);
        @display("i=block/inserter");
}
