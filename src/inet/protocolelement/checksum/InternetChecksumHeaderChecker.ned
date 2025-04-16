//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.ChecksumHeaderChecker;

//
// Checks Internet checksum headers in packets to verify data
// integrity. This module extends ChecksumHeaderChecker and
// specializes it to use the "internet" checksum type.
//
// @see InternetChecksumHeaderInserter
//
simple InternetChecksumHeaderChecker extends ChecksumHeaderChecker
{
    parameters:
        checksumType = "internet";
}
