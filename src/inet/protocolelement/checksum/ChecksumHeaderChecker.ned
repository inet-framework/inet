//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.ChecksumCheckerBase;

simple ChecksumHeaderChecker extends ChecksumCheckerBase
{
    parameters:
        string headerPosition @enum("front","back") = default("front");
        @class(ChecksumHeaderChecker);
}
