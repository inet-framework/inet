//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.scheduler;

import inet.queueing.base.PacketSchedulerBase;
import inet.queueing.contract.IPacketScheduler;

//
// Extensible packet scheduler that uses a pluggable scheduling function to
// select which input gate to pull packets from. This provides a framework for
// implementing custom scheduling algorithms.
//
// Operation mechanism:
// - When a packet is requested, the scheduler invokes the configured scheduler function
// - The function examines the available packet providers and returns an integer
// - This integer determines which input gate to pull a packet from
//
// The PacketScheduler serves as a base for implementing custom scheduling algorithms
// without having to implement a complete scheduler module. By providing different
// scheduler function implementations, various scheduling policies can be realized.
//
// The schedulerClass parameter specifies the C++ class that implements the
// scheduling algorithm. This class must implement the IPacketSchedulerFunction
// interface and be registered via Register_Class.
//
// @see ~PriorityScheduler, ~WrrScheduler, ~MarkovScheduler
//
simple PacketScheduler extends PacketSchedulerBase like IPacketScheduler
{
    parameters:
        string schedulerClass; // Determines the input queue of the packet, the parameter must be the name of a C++ class which implements the IPacketSchedulerFunction interface and is registered via Register_Class
        @class(PacketScheduler);
}
