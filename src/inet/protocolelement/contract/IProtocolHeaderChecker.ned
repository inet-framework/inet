//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.contract;

import inet.queueing.contract.IPacketFilter;

//
// Defines the interface for protocol header checker modules that validate
// packet headers according to protocol specifications and filter out invalid packets.
//
// Operation:
// - Examines protocol-specific header fields in packets
// - Verifies header field values against protocol requirements
// - Validates checksums, sequence numbers, or other integrity mechanisms
// - Passes valid packets to upper layers
// - Filters out packets with invalid headers
//
// @see ~IProtocolHeaderInserter, ~CrcHeaderChecker, ~FcsHeaderChecker
//
moduleinterface IProtocolHeaderChecker extends IPacketFilter
{
    parameters:
        @display("i=block/checker");
}
