//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.ChecksumHeaderInserter;

//
// Inserts CRC-32 (Cyclic Redundancy Check) headers into packets to enable data
// integrity verification. This module extends ChecksumHeaderInserter and
// specializes it to use the "crc32" checksum type.
//
// @see Crc32HeaderChecker
//
simple Crc32HeaderInserter extends ChecksumHeaderInserter
{
    parameters:
        checksumType = "ethernet-fcs";
}
