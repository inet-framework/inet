//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.shaper;

import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketScheduler;
import inet.queueing.contract.IPacketShaper;
import inet.queueing.queue.CompoundPacketQueueBase;

//
// Implements a priority-based traffic shaping mechanism that classifies incoming
// packets into multiple traffic classes, applies separate shaping to each class,
// and then schedules packets according to their priority.
//
// The priority shaper architecture consists of three main components:
// - A classifier that directs incoming packets to appropriate traffic shapers
// - Multiple traffic shapers (one per traffic class) that control the rate of each flow
// - A scheduler that selects packets from the shapers based on priority
//
// Key features:
// - Supports multiple traffic classes with different QoS requirements
// - Applies independent traffic shaping to each class
// - Prioritizes higher-priority traffic over lower-priority traffic
// - Configurable number of shapers for different traffic classes
// - Customizable classifier and scheduler components
//
// Applications:
// - Differentiated Services (DiffServ) implementation
// - Quality of Service (QoS) enforcement
// - Traffic management in converged networks
// - Multi-level traffic conditioning
// - Hierarchical bandwidth allocation
//
// The priority shaper is particularly useful in scenarios where:
// - Different traffic types require different treatment
// - Both rate limiting and prioritization are needed
// - Complex traffic management policies must be implemented
// - Network resources need to be allocated based on traffic importance
//
// Typical configuration uses a PriorityScheduler that always serves the
// highest priority non-empty shaper first, but other schedulers like
// WrrScheduler can be used for more sophisticated bandwidth sharing.
//
// @see ~PacketShaper, ~PriorityScheduler, ~WrrScheduler
//
module PriorityShaper extends CompoundPacketQueueBase like IPacketShaper
{
    parameters:
        int numShapers; // Number of traffic shapers (one per traffic class)
    submodules:
        classifier: <default("PacketClassifier")> like IPacketClassifier {
            @display("p=100,150");
        }
        shaper[numShapers]: <default("PacketShaper")> like IPacketShaper {
            @display("p=300,150,column,150");
        }
        scheduler: <default("PriorityScheduler")> like IPacketScheduler {
            @display("p=500,150");
        }
    connections:
        in --> classifier.in;
        for i=0..numShapers-1 {
            classifier.out++ --> shaper[i].in;
            shaper[i].out --> scheduler.in++;
        }
        scheduler.out --> out;
}
