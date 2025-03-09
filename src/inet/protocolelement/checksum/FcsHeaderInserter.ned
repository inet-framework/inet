//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.FcsInserterBase;

//
// Calculates and inserts Frame Check Sequence (FCS) headers into packets
// to enable integrity verification during transmission at the data link layer.
//
// Operation:
// - Calculates 32-bit FCS values for packet content
// - Creates FCS headers with the computed values
// - Inserts headers at specified position (typically at the back)
// - Supports multiple FCS modes: disabled, declared, computed
// - Registers with the AccessoryProtocol service
//
// @see ~FcsHeaderChecker, ~CrcHeaderInserter, ~FcsInserterBase
//
simple FcsHeaderInserter extends FcsInserterBase
{
    parameters:
        string headerPosition @enum("front","back") = default("back");
        @class(FcsHeaderInserter);
}
