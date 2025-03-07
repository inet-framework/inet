//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

//
// Packet queue with a limited capacity that drops packets at the head of the queue
// when it becomes full. This queue management strategy provides different behavior
// compared to the more common drop-tail approach.
//
// When the queue reaches its capacity limit (packetCapacity), any newly arriving
// packet causes the oldest packet (the one at the head/beginning of the queue) to
// be dropped. This behavior is implemented using the PacketAtCollectionBeginDropper.
//
// Drop-head queuing has several distinctive characteristics:
// - Favors newer packets over older ones, which can be beneficial for real-time traffic
// - Helps prevent head-of-line blocking in certain scenarios
// - Can provide better quality of service for time-sensitive applications
// - May improve performance for applications where newer data is more valuable than older data
//
// This queue is particularly useful in scenarios like:
// - Real-time multimedia streaming where newer frames are more important
// - Control systems where the most recent command should take precedence
// - Applications where stale data has diminished value
//
// @see ~DropTailQueue, ~PacketQueue, ~PacketAtCollectionBeginDropper
//
simple DropHeadQueue extends PacketQueue
{
    parameters:
        packetCapacity = default(100);
        dropperClass = default("inet::queueing::PacketAtCollectionBeginDropper");
}
