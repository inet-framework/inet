//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

//
// Packet queue with a limited capacity that drops packets at the tail of the queue
// when it becomes full. This is one of the most commonly used queue management
// strategies in network devices.
//
// When the queue reaches its capacity limit (packetCapacity), any newly arriving
// packet causes the most recently arrived packet (the one at the tail/end of the
// queue) to be dropped. This behavior is implemented using the PacketAtCollectionEndDropper.
//
// Drop-tail queuing is simple to implement but can lead to issues like:
// - Global synchronization in TCP flows (multiple TCP connections backing off simultaneously)
// - Potential unfairness between flows
// - No differentiation between different types of traffic
//
// This queue is commonly used in network interfaces, routers, and switches where
// simplicity is valued over sophisticated queue management.
//
// @see ~DropHeadQueue, ~PacketQueue, ~PacketAtCollectionEndDropper
//
simple DropTailQueue extends PacketQueue
{
    parameters:
        packetCapacity = default(100);
        dropperClass = default("inet::queueing::PacketAtCollectionEndDropper");
}
