//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// Packet scheduler that pulls packets from the first non-empty provider among
// its connected packet providers, implementing a strict priority scheduling
// mechanism.
//
// The PriorityScheduler assigns fixed priorities to its inputs based on their
// index, with lower indices having higher priority. When a packet is requested,
// the scheduler checks each provider in order (starting from index 0) and pulls
// a packet from the first one that has a packet available.
//
// This scheduler is commonly used in:
// - Quality of Service (QoS) implementations to prioritize certain traffic types
// - Network devices that need to handle high-priority control traffic before data traffic
// - Multi-level priority queuing systems
// - Implementing strict priority scheduling in routers and switches
//
// The scheduler does not preempt packets that are already being transmitted,
// but ensures that higher priority packets are selected next when the current
// transmission completes.
//
// @see ~PriorityQueue, ~WrrScheduler, ~MarkovScheduler
//
simple PriorityScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        @class(PriorityScheduler);
}
