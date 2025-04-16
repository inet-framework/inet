//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.ChecksumHeaderInserter;

//
// Inserts Internet checksum headers into packets to enable data integrity
// verification. This module extends ChecksumHeaderInserter and specializes
// it to use the "internet" checksum type.
//
// @see InternetChecksumHeaderChecker
//
simple InternetChecksumHeaderInserter extends ChecksumHeaderInserter
{
    parameters:
        checksumType = "internet";
}
