//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFilterBase;

simple CrcCheckerBase extends PacketFilterBase
{
    parameters:
        @class(CrcCheckerBase);
        @display("i=block/checker");
}
