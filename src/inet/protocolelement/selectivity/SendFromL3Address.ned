//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.queueing.base.PacketFlowBase;

//
// Prepends a source L3 address header carrying the configured ~address, so a peer
// running ~ReceiveFromL3Address can learn who sent the packet. The source-address
// counterpart of ~SendToL3Address.
//
simple SendFromL3Address extends PacketFlowBase
{
    parameters:
        @class(SendFromL3Address);
        string address; // this node's L3 address, stamped into the source address header
        @display("i=block/tx");
}
