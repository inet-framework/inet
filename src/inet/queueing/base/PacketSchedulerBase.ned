//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet scheduler components in the queueing framework.
// Packet schedulers select packets from multiple inputs based on specific
// scheduling algorithms, providing traffic management capabilities.
//
// Schedulers typically operate by:
// 1. Receiving pull requests from their output
// 2. Selecting one of their inputs based on a scheduling algorithm
// 3. Pulling a packet from the selected input
// 4. Forwarding the packet to their output
//
// The scheduling decision can be based on various criteria:
// - Static priorities (always prefer certain inputs)
// - Dynamic priorities (based on current system state)
// - Round-robin (serve inputs in circular order)
// - Weighted fair queueing (proportional service)
// - Content-based (using packet properties)
//
// Derived scheduler modules implement various scheduling disciplines including:
// - Priority scheduling (strict priority order)
// - Weighted round-robin (WRR) scheduling
// - Deficit round-robin (DRR) scheduling
// - Weighted fair queueing (WFQ)
// - Content-based scheduling
//
// The reverseOrder parameter can be used to invert the priority order
// of inputs when using priority-based scheduling algorithms.
//
// @see ~IPacketScheduler, ~PriorityScheduler, ~WrrScheduler, ~LabelScheduler
//
simple PacketSchedulerBase extends PacketProcessorBase
{
    parameters:
        bool reverseOrder = default(false); // When true, reverses the priority order of inputs for priority-based schedulers
        displayStringTextFormat = default("scheduled %p pk (%l)");
        @class(PacketSchedulerBase);
        @display("i=block/join");
        @signal[packetPulled](type=inet::Packet);
        // the statistical value is the scheduled packet
        @statistic[packets](title="packets"; source=packetPulled; record=count; unit=pk);
        // the statistical value is the length of the scheduled packet
        @statistic[packetLengths](title="packet lengths"; source=packetLength(packetPulled); record=sum,histogram,vector; unit=b; interpolationmode=none);
        // the statistical value is the data rate of the scheduled packets
        @statistic[dataRate](title="data rate"; source=throughput(packetPulled); record=vector; unit=bps; interpolationmode=linear);
    gates:
        input in[];
        output out;
}
