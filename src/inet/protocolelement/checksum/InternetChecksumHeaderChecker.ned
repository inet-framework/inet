//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.ChecksumHeaderInserter;

simple InternetChecksumHeaderChecker extends ChecksumHeaderInserter
{
    parameters:
        checksumType = "internet";
        string headerPosition @enum("front","back") = default("front");
        @class(CrcHeaderInserter);
}
