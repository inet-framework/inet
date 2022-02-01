//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketDuplicatorBase;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPassivePacketSink;

//
// This module connects one packet producer to one packet consumer. It can be
// pushed with packets from the connected packet producer. When this happens,
// one or more copy of the packet is pushed to the output gate.
//
simple PacketDuplicator extends PacketDuplicatorBase like IPassivePacketSink, IActivePacketSource
{
    parameters:
        volatile int numDuplicates = default(1); // number of duplicates, 0 means only the original packet is forwarded
        @class(PacketDuplicator);
        @display("i=block/star");
    gates:
        input in @labels(push);
        output out @labels(push);
}
