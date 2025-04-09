//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.ChecksumHeaderChecker;

simple Crc32HeaderChecker extends ChecksumHeaderChecker
{
    parameters:
        checksumType = "crc32";
}
