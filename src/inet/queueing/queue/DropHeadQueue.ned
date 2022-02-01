//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

//
// This is a limited packet queue which drops packets at the head of the queue.
//
simple DropHeadQueue extends PacketQueue
{
    parameters:
        packetCapacity = default(100);
        dropperClass = default("inet::queueing::PacketAtCollectionBeginDropper");
}
