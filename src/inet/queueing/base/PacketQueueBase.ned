//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.base;

//
// Base module for packet queue components in the queueing framework.
// Packet queues store packets temporarily during processing, providing
// buffering capabilities essential for network devices.
//
// Packet queues are fundamental building blocks for:
// - Network interface output buffers
// - Router and switch port queues
// - Traffic management systems
// - Protocol stacks with layered buffering
// - Any system requiring temporary packet storage
//
// Queues typically operate in a passive mode where:
// - Other modules push packets into the queue
// - Other modules pull packets from the queue
// - The queue itself doesn't initiate packet movement
//
// Derived queue modules implement various queueing disciplines including:
// - FIFO (First-In-First-Out)
// - Priority-based queueing
// - Weighted fair queueing
// - Random early detection
// - Custom queueing strategies
//
// The module tracks comprehensive statistics about queue operations:
// - Current number of packets and total length
// - Number of pushed, pulled, removed, and dropped packets
// - Queue occupancy over time
//
// Available display string format directives:
// - %p - number of packets
// - %l - total length
// - %u - number of pushed packets
// - %o - number of pulled packets
// - %r - number of removed packets
// - %d - number of dropped packets
// - %n - name of the next pulled packet
//
// @see ~IPacketQueue, ~PacketQueue, ~DropTailQueue, ~PriorityQueue
//
simple PacketQueueBase extends PacketProcessorBase
{
    parameters:
        displayStringTextFormat = default("contains %p pk (%l)\npushed %u pulled %o dropped %d"); // Determines display string text above the submodule
        @display("i=block/queue");
    gates:
        input in @loose;
        output out @loose;
}
