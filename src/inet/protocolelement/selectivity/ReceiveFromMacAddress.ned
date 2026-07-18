//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.queueing.base.PacketFlowBase;

//
// Pops the source MAC address header inserted by ~SendFromMacAddress and stores the sender's MAC in
// a MacAddressInd tag, so ~ReceiveWithAcknowledge can unicast its acknowledgement back to the sender.
// Never drops the packet.
//
simple ReceiveFromMacAddress extends PacketFlowBase
{
    parameters:
        @class(ReceiveFromMacAddress);
        @display("i=block/rx");
}
