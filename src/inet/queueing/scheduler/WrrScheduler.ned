//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// Weighted Round-Robin (WRR) packet scheduler that allocates bandwidth
// proportionally to configured weights. This provides fair sharing of
// resources while preventing starvation of lower-priority traffic.
//
// Operation mechanism:
// - Each input gate has an associated weight specified in the weights parameter
// - Initially, each input gate receives tokens equal to its weight
// - When a packet is requested, the scheduler selects the first input gate that:
//   1. Has a packet available
//   2. Has at least one token
// - One token is consumed when a packet is pulled from an input gate
// - When packets are waiting but no gate has enough tokens, all token buckets are refilled
//
// The WRR scheduler is commonly used in:
// - Quality of Service (QoS) implementations requiring fair bandwidth allocation
// - Network devices that need to balance traffic from multiple sources
// - Systems where strict priority scheduling would cause starvation
// - Implementing Class-Based Queuing (CBQ) and other fair queuing mechanisms
//
// The weights parameter specifies relative bandwidth allocation as a comma-separated
// list of integers, with one weight per input gate.
//
// @see ~PriorityScheduler, ~MarkovScheduler
//
simple WrrScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        string weights;
        @class(WrrScheduler);
}
