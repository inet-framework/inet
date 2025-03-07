//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.contract;

//
// Contract for packet queue implementations
// in the queueing framework. A packet queue is a fundamental component that
// temporarily stores packets while they await processing or transmission.
//
// Key characteristics:
// - Acts as both a passive packet sink and a passive packet source
// - Receives packets through its passive input (pushed by other modules)
// - Provides packets through its passive output (pulled by other modules)
// - Implements various queueing disciplines and packet management strategies
// - May drop packets based on capacity limits or other criteria
//
// Packet queues are essential components in network devices, implementing
// various queueing disciplines such as:
// - FIFO (First-In-First-Out): packets depart in the same order they arrive
// - LIFO (Last-In-First-Out): most recently arrived packets depart first
// - Priority queuing: packets with higher priority depart before lower priority ones
// - Fair queueing: bandwidth is shared fairly among different flows
// - Random Early Detection (RED): packets are dropped probabilistically as queue fills
//
// Common implementations include DropTailQueue, DropHeadQueue, PriorityQueue,
// and RedMarkerQueue. When a queue is not needed in a model, OmittedPacketQueue
// can be used as a placeholder.
//
moduleinterface IPacketQueue extends IPassivePacketSink, IPassivePacketSource
{
    parameters:
        @omittedTypename(OmittedPacketQueue);
        @display("i=block/queue");
}
