//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.ChecksumCheckerBase;

//
// Checks the checksum values in packet headers to verify data integrity. 
// This module extends ChecksumCheckerBase and adds the ability to specify
// the position of the checksum header (front or back of the packet). It processes
// packets by popping the checksum header, checking if the checksum value matches the
// expected value based on the checksum mode, and filtering out packets with invalid
// checksums.
//
// @see ChecksumHeaderInserter
//
simple ChecksumHeaderChecker extends ChecksumCheckerBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(ChecksumHeaderChecker);
}
