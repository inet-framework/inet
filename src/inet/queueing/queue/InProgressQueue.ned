//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

//
// Specialized packet queue that only accepts packets with higher priority than
// those already in the queue. Unlike standard packet queues, it always returns
// false for canPushSomePacket() and only allows insertion of packets that
// compare favorably against the first packet in the queue according to the
// configured comparator function (see comparatorClass parameter). Useful for 
// implementing priority-based packet processing where only higher priority
// packets can preempt currently queued packets.
//
simple InProgressQueue extends PacketQueue
{
    parameters:
        @class(InProgressQueue);
}
