//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.checksum.base;

import inet.queueing.base.PacketFilterBase;

//
// TODO
//
simple FcsCheckerBase extends PacketFilterBase
{
    parameters:
        @class(FcsCheckerBase);
        @display("i=block/checker");
}
