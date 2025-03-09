//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFilterBase;

//
// Base module for CRC (Cyclic Redundancy Check) verification in network protocols.
// Provides core functionality for validating packet integrity using CRC checksums.
//
// Operation:
// - Examines packets for CRC checksums
// - Verifies checksums against packet content
// - Supports multiple CRC modes: disabled, declared correct/incorrect, computed
// - Filters out packets with invalid checksums
// - Passes through packets with valid checksums
//
// @see ~CrcInserterBase, ~FcsCheckerBase, ~PacketFilterBase
//
simple CrcCheckerBase extends PacketFilterBase
{
    parameters:
        @class(CrcCheckerBase);
        @display("i=block/checker");
}
