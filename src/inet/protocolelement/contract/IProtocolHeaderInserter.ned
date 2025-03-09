//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.contract;

import inet.queueing.contract.IPacketFlow;

//
// Defines the interface for protocol header inserter modules that add
// protocol-specific headers to packets before transmission.
//
// Operation:
// - Creates protocol-specific headers for outgoing packets
// - Populates header fields according to protocol specifications
// - Computes and inserts checksums, sequence numbers, or other integrity mechanisms
// - Attaches headers to packets in the correct format
// - Forwards packets with headers to lower layers
//
// @see ~IProtocolHeaderChecker, ~CrcHeaderInserter, ~FcsHeaderInserter
//
moduleinterface IProtocolHeaderInserter extends IPacketFlow
{
    parameters:
        @display("i=block/inserter");
}
