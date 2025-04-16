//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.ChecksumHeaderInserter;

simple Crc32HeaderInserter extends ChecksumHeaderInserter
{
    parameters:
        checksumType = "ethernet-fcs";
}
