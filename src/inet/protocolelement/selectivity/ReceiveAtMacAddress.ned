//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.queueing.base.PacketFilterBase;

simple ReceiveAtMacAddress extends PacketFilterBase
{
    parameters:
        @class(ReceiveAtMacAddress);
        string address;
        @display("i=block/rx");
}
