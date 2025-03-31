//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSource;

//
// Adapter module that converts packet sending to packet pushing. When it
// receives a packet as a message through its input gate, it pushes the packet
// to the connected consumer module through its output gate. Serves as a bridge
// between different packet handling mechanisms in protocol stacks and packet
// processing pipelines.
//
simple PacketSendToPush extends PacketProcessorBase like IActivePacketSource
{
    parameters:
        @class(PacketSendToPush);
        @display("i=block/cogwheel");
    gates:
        input in @labels(push);
        output out @labels(push);
}
