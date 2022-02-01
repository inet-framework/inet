//
// Copyright (C) 2019 Marcel Marek
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.queue;

import inet.queueing.contract.IPacketQueue;
import inet.queueing.filter.RedDropper;

//
// Combines a packet queue with random early detection (RED) dropper.
//
module RedDropperQueue extends CompoundPacketQueueBase
{
    submodules:
        red: RedDropper {
            @display("p=100,100");
        }
        fifo: <default("PacketQueue")> like IPacketQueue {
            @display("p=100,200");
        }
    connections:
        in --> { @display("m=n"); } --> red.in;
        red.out --> fifo.in;
        fifo.out --> { @display("m=s"); } --> out;
}
