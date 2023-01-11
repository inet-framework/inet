//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum;

import inet.protocolelement.checksum.base.FcsInserterBase;

simple FcsHeaderInserter extends FcsInserterBase
{
    parameters:
        string headerPosition @enum("front","back") = default("back");
        @class(FcsHeaderInserter);
}
