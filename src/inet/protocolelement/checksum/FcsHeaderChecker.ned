//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.FcsCheckerBase;

simple FcsHeaderChecker extends FcsCheckerBase
{
    parameters:
        string headerPosition @enum("front","back") = default("back");
        @class(FcsHeaderChecker);
}
