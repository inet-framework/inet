//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// This scheduler pulls packets from the first non-empty among its connected
// packet providers.
//
simple PriorityScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        @class(PriorityScheduler);
}
