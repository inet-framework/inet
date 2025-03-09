//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFilterBase;

//
// Base module for Frame Check Sequence (FCS) verification in data link layer
// protocols. Provides functionality for validating frame integrity using
// 32-bit CRC checksums.
//
// Operation:
// - Examines frames for FCS fields
// - Verifies FCS values against frame content
// - Supports multiple FCS modes: disabled, declared correct/incorrect, computed
// - Filters out frames with invalid FCS values
// - Passes through frames with valid FCS values
//
// @see ~FcsInserterBase, ~CrcCheckerBase, ~PacketFilterBase
//
simple FcsCheckerBase extends PacketFilterBase
{
    parameters:
        @class(FcsCheckerBase);
        @display("i=block/checker");
}
