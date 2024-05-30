//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IPacketFlow;

// This module takes packets passed to its input and streams them to its output.
//
simple PacketStreamer extends PacketProcessorBase like IPacketFlow
{
    parameters:
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
        double datarate @unit(bps) = default(nan bps);
        @class(PacketStreamer);
        @display("i=block/cogwheel");
    gates:
        input in @interface[inet::queueing::IPassivePacketSink] @interface[inet::queueing::IActivePacketSink] @labels(send,push,pull,pass);
        output out @interface[inet::queueing::IPassivePacketSource] @interface[inet::queueing::IActivePacketSource] @labels(send,push,pull,stream);
}
