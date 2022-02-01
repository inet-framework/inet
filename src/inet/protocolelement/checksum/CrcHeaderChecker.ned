//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.CrcCheckerBase;

simple CrcHeaderChecker extends CrcCheckerBase
{
    parameters:
        string headerPosition @unit("front","back") = default("front");
        @class(CrcHeaderChecker);
}
