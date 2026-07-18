//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.queueing.base.PacketFlowBase;

//
// Pops the source L3 address header inserted by ~SendFromL3Address and stores the sender's
// address in an L3AddressInd tag for higher elements. Never drops the packet.
//
simple ReceiveFromL3Address extends PacketFlowBase
{
    parameters:
        @class(ReceiveFromL3Address);
        @display("i=block/rx");
}
