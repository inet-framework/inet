//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.queueing.base.PacketFlowBase;

//
// Prepends a source MAC address header carrying the configured ~address, so a peer running
// ~ReceiveFromMacAddress can learn who sent the frame (used to unicast acknowledgements back on a
// shared medium). The source-MAC counterpart of ~SendToMacAddress.
//
simple SendFromMacAddress extends PacketFlowBase
{
    parameters:
        @class(SendFromMacAddress);
        string address; // this interface's MAC address, stamped into the source MAC header
        @display("i=block/tx");
}
