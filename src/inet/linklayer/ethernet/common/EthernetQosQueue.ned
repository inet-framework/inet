//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.filter.RedDropper;
import inet.queueing.queue.CompoundPacketQueueBase;
import inet.queueing.queue.DropTailQueue;

//
// Specialized queue for Ethernet frames that prioritizes frames based on their type. Uses a comparator 
// to order frames, typically giving PAUSE frames higher priority than data frames to ensure timely 
// processing of flow control messages.
//
// @see ~EthernetQosQueue, ~EthernetQosRedQueue, ~DropTailQueue
//
//# TODO rename to `EthernetQosQueue` or keep name as it is?
module EthernetQueue extends DropTailQueue
{
    parameters:
        comparatorClass = default("inet::EthernetFrameTypeComparator");
}

//
// Compound queue module that implements Quality of Service for Ethernet frames. Separates PAUSE frames 
// from data frames using a classifier and processes them through separate queues with different priorities. 
// Ensures flow control frames receive higher priority treatment while allowing customization of the data 
// queue implementation.
//
// @see ~EthernetQueue, ~EthernetQosRedQueue, ~IPacketQueue, ~EthernetFrameClassifier
//
//# TODO rename to `CompoundEtherQueue`? or `CompoundEtherQosQueue`?
module EthernetQosQueue extends CompoundPacketQueueBase like IPacketQueue
{
    parameters:
        @display("i=block/queue");
    submodules:
        classifier: <default("EthernetFrameClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,200");
        }
        pauseQueue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=250,100");
        }
        dataQueue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=250,300;q=l2queue");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                @display("p=400,200");
        }
    connections:
        in --> classifier.in;
        classifier.out++ --> pauseQueue.in;
        classifier.out++ --> dataQueue.in;
        pauseQueue.out --> scheduler.in++;
        dataQueue.out --> scheduler.in++;
        scheduler.out --> out;
}

//
// Enhanced QoS queue for Ethernet that combines priority handling with congestion management. Separates 
// PAUSE frames for high-priority processing while applying Random Early Detection (RED) to data frames 
// to prevent queue overflow. Provides congestion control by selectively dropping packets based on queue 
// occupancy before the queue becomes full.
//
// @see ~EthernetQueue, ~EthernetQosQueue, ~RedDropper, ~IPacketQueue
//
module EthernetQosRedQueue extends CompoundPacketQueueBase like IPacketQueue
{
    parameters:
        @display("i=block/queue");
    submodules:
        classifier: <default("EthernetFrameClassifier")> like IPacketClassifier {
            parameters:
                @display("p=100,200");
        }
        pauseQueue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=250,100");
        }
        redFilter: RedDropper {
            @display("p=190,300");
        }
        dataQueue: <default("DropTailQueue")> like IPacketQueue {
            parameters:
                @display("p=310,300;q=l2queue");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            parameters:
                @display("p=400,200");
        }
    connections:
        in --> classifier.in;
        classifier.out++ --> pauseQueue.in;
        classifier.out++ --> redFilter.in;
        pauseQueue.out --> scheduler.in++;
        dataQueue.out --> scheduler.in++;
        scheduler.out --> out;
        redFilter.out --> dataQueue.in;
}
