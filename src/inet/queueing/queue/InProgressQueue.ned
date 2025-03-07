//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

//
// Specialized packet queue that tracks packets currently being processed by
// a server or other network element. This queue maintains references to packets
// that have been dequeued but are still being processed elsewhere in the system.
//
// This queue is often used in conjunction with servers that have non-zero
// processing times, allowing the system to keep track of packets that have
// been dequeued but have not yet completed processing.
//
// @see ~PacketQueue, ~PacketServer
//
simple InProgressQueue extends PacketQueue
{
    parameters:
        @class(InProgressQueue);
}
