//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IActivePacketSink;

//
// This module connects multiple packet collectors to one packet provider. When
// a packet is pulled by one of the connected packet collectors, then the demultiplexer
// simply pulls a packet from the connected packet provider.
//
simple PacketDemultiplexer extends PacketProcessorBase like IActivePacketSink // TODO , IPassivePacketSource // out gate is vector
{
    parameters:
        displayStringTextFormat = default("passed %p pk (%l)");
        @class(PacketDemultiplexer);
        @display("i=block/fork");
    gates:
        input in @labels(pull);
        output out[] @labels(pull);
}
