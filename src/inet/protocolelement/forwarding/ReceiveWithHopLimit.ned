//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.forwarding;

import inet.queueing.base.PacketFilterBase;

simple ReceiveWithHopLimit extends PacketFilterBase
{
    parameters:
        @class(ReceiveWithHopLimit);
        @display("i=block/rx");
}
