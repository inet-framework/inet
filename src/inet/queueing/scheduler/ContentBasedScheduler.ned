//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// This module connects one packet collector to multiple packet providers. It
// can be pulled for packets by the connected packet collector. When this
// happens, the scheduler pulls a packet from one of its packet providers based
// on the configured packet filters. The first matching expression determines
// the index of the input gate.
//
simple ContentBasedScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        int defaultGateIndex = default(0); // default gate index if no matching labels are found
        object packetFilters; // array of packet filter expressions
        @class(ContentBasedScheduler);
}
