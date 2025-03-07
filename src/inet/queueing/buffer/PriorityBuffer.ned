//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.buffer;

//
// Priority-based packet buffer that selectively drops
// packets based on the priority of their owner queues. When the buffer becomes
// overloaded, it drops packets from the queue with the highest module ID first.
//
// The PriorityBuffer is a specialized version of PacketBuffer that uses the
// PacketWithHighestOwnerModuleIdDropper strategy by default. This implements
// a simple priority scheme where queues with lower module IDs have higher
// priority (their packets are less likely to be dropped).
//
// Key features:
// - Fixed packet capacity of 100 packets by default
// - Prioritizes packets based on the module ID of their owner queue
// - Drops packets from lower priority queues (higher module IDs) first
// - Useful for implementing basic quality of service mechanisms
//
// @see ~PacketBuffer
//
simple PriorityBuffer extends PacketBuffer
{
    parameters:
        packetCapacity = default(100);
        dropperClass = default("inet::queueing::PacketWithHighestOwnerModuleIdDropper");
}
