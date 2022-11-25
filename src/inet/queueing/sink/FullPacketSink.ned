//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.sink;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSink;
import inet.queueing.contract.IPacketSink;
import inet.queueing.contract.IPassivePacketSink;

//
// This module is an active/passive full packet sink. It doesn't pull packets
// from the connected module, and the it doesn't allow packets to be pusehd by
// the connected module. The input gate generates backpressure for push packet
// operations.
//
simple FullPacketSink extends PacketProcessorBase like IPacketSink, IActivePacketSink, IPassivePacketSink
{
    parameters:
        @class(FullPacketSink);
        @display("i=block/sink");
    gates:
        input in @labels(push,pull);
}
