//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.ChecksumHeaderChecker;

//
// Checks CRC-32 (Cyclic Redundancy Check) headers in packets to verify data
// integrity. This module extends ChecksumHeaderChecker and specializes it
// to use the "crc32" checksum type.
//
// @see Crc32HeaderInserter
//
simple Crc32HeaderChecker extends ChecksumHeaderChecker
{
    parameters:
        checksumType = "crc32";
}
