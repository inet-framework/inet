//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.authentication;

import inet.protocolelement.contract.IProtocolHeaderChecker;
import inet.queueing.base.PacketFilterBase;

//
// Verifies message authentication codes (MACs) in packet headers to ensure
// data integrity and authenticity in network communications.
//
// Operation:
// - Extracts the MAC from incoming packet headers
// - Verifies the MAC against expected values
// - Passes authentic packets to upper layers
// - Drops packets with invalid authentication codes
// - Provides security against tampering and forgery
//
// @see ~MessageAuthenticationCodeInserter, ~PacketFilterBase
//
simple MessageAuthenticationCodeChecker extends PacketFilterBase like IProtocolHeaderChecker
{
    parameters:
        int headerLength @unit(b) = default(4B);
        @class(MessageAuthenticationCodeChecker);
        @display("i=block/checker");
}
