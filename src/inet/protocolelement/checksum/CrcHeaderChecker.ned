//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.CrcCheckerBase;

//
// Verifies CRC (Cyclic Redundancy Check) values in packet headers and
// removes the CRC header if valid. Filters out packets with invalid CRC.
//
// Operation:
// - Examines CRC headers at specified position (front or back)
// - Verifies CRC values against packet content
// - Removes CRC headers from valid packets
// - Drops packets with invalid CRC values
// - Registers with the AccessoryProtocol service
//
// @see ~CrcHeaderInserter, ~FcsHeaderChecker, ~CrcCheckerBase
//
simple CrcHeaderChecker extends CrcCheckerBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(CrcHeaderChecker);
}
