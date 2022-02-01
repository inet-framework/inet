//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.common;

import inet.queueing.base.PacketProcessorBase;
import inet.queueing.contract.IPacketDelayer;

//
// This module connects one packet producer to one packet consumer. It can be
// pushed with packets from the connected packet producer. When this happens,
// the packet is delayed and eventually pushed to the output. Note that the
// order of packets may change if the delay parameter is configured to be a
// distribution.
//
simple PacketDelayer extends PacketProcessorBase like IPacketDelayer
{
    parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        volatile double delay @unit(s);
        @class(PacketDelayer);
        @display("i=block/delay");
    gates:
        input in @labels(push);
        output out @labels(push);
}
