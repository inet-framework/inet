//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.selectivity;

import inet.queueing.base.PacketFlowBase;

//
// Prepends a source port header carrying the configured ~port, so a peer running
// ~ReceiveFromPort can learn the sender's port. The source-port counterpart of ~SendToPort.
//
simple SendFromPort extends PacketFlowBase
{
    parameters:
        @class(SendFromPort);
        int port; // the sender's port, stamped into the source port header
        @display("i=block/tx");
}
