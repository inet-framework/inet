//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFlowBase;

//
// Base module for Frame Check Sequence (FCS) calculation and insertion
// in data link layer protocols. Provides functionality for adding 32-bit
// CRC checksums to frames for integrity verification.
//
// Operation:
// - Calculates 32-bit FCS values for frames
// - Supports multiple FCS modes: disabled, declared, computed
// - Inserts FCS values into frames according to derived class specifications
// - Uses Ethernet CRC-32 polynomial for computation
// - Processes frames without modifying their core content
//
// @see ~FcsCheckerBase, ~CrcInserterBase, ~PacketFlowBase
//
simple FcsInserterBase extends PacketFlowBase
{
    parameters:
        string fcsMode @enum("disabled","declared","computed") = default("declared");
        @class(FcsInserterBase);
        @display("i=block/inserter");
}
