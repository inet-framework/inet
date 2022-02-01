//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IPacketFlow;

//
// This module takes packets streamed to its input and passes them to its output.
//
simple PacketDestreamer extends PacketProcessorBase like IPacketFlow
{
    parameters:
        double datarate @unit(bps) = default(nan bps);
        @class(PacketDestreamer);
        @display("i=block/cogwheel");
    gates:
        input in @labels(send,push,pull,stream);
        output out @labels(send,push,pull,pass);
}
