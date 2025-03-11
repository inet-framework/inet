//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.CrcInserterBase;

//
// Calculates and inserts CRC (Cyclic Redundancy Check) headers into packets
// to enable integrity verification during transmission.
//
// Operation:
// - Calculates CRC values for packet content
// - Creates CRC headers with the computed values
// - Inserts headers at specified position (front or back)
// - Supports multiple CRC modes: disabled, declared, computed
// - Registers with the AccessoryProtocol service
//
// Particularly useful for:
// - Adding integrity checking to custom protocols
// - Implementing protocol-specific CRC generation
// - Creating realistic protocol behavior with error detection
// - Building protocol stacks with header-based checksums
// - Testing error detection mechanisms in network simulations
//
// @see ~CrcHeaderChecker, ~FcsHeaderInserter, ~CrcInserterBase
//
simple CrcHeaderInserter extends CrcInserterBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(CrcHeaderInserter);
}
