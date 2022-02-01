//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IPassivePacketSink;

//
// This module connects one packet producer to one packet consumer. It can be
// pushed with packets from the connected packet producer. When this happens,
// a separate copy of the packet is pushed to each output gate.
//
simple PacketCloner extends PacketProcessorBase like IPassivePacketSink // IActivePacketSource // TODO out gate is vector
{
    parameters:
        @class(PacketCloner);
        @display("i=block/star");
    gates:
        input in @labels(push);
        output out[] @labels(push);
}
