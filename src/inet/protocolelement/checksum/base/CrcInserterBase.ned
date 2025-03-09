//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFlowBase;

//
// Base module for CRC (Cyclic Redundancy Check) calculation and insertion
// in network protocols. Provides core functionality for adding integrity
// verification to packets.
//
// Operation:
// - Calculates CRC checksums for packets
// - Supports multiple CRC modes: disabled, declared, computed
// - Inserts checksums into packets according to derived class specifications
// - Processes packets without modifying their core content
// - Maintains packet flow while adding error detection capability
//
// Particularly useful for:
// - Implementing error detection in protocol simulations
// - Creating realistic protocol behavior with integrity checks
// - Supporting various checksum calculation methods
// - Building protocol stacks with error detection capabilities
// - Testing network resilience with different CRC modes
//
// @see ~CrcCheckerBase, ~FcsInserterBase, ~PacketFlowBase
//
simple CrcInserterBase extends PacketFlowBase
{
    parameters:
        string crcMode @enum("disabled","declared","computed") = default("declared");
        @class(CrcInserterBase);
        @display("i=block/inserter");
}
