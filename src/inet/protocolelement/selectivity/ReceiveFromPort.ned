//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.queueing.base.PacketFlowBase;

//
// Pops the source port header inserted by ~SendFromPort and stores the sender's port in an
// L4PortInd tag for higher elements. Never drops the packet.
//
simple ReceiveFromPort extends PacketFlowBase
{
    parameters:
        @class(ReceiveFromPort);
        @display("i=block/rx");
}
