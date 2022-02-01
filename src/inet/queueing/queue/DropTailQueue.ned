//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

//
// This module is a limited packet queue which drops packets at the tail of the queue.
//
simple DropTailQueue extends PacketQueue
{
    parameters:
        packetCapacity = default(100);
        dropperClass = default("inet::queueing::PacketAtCollectionEndDropper");
}
