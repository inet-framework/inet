//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.queueing.source;

import inet.queueing.contract.IActivePacketSource;
import inet.queueing.scheduler.MarkovScheduler;

//
// Generates realistic bursty network traffic by combining two packet sources
// with different characteristics. Uses a Markov-based scheduler to switch
// between normal and burst modes, creating traffic patterns that exhibit both
// sustained background traffic and intermittent high-volume bursts.
//
// Key features:
// - Combines two independent packet sources with different configurations
// - Uses a Markov scheduler to dynamically switch between sources
// - Can model various bursty traffic patterns by adjusting source parameters
// - Produces more realistic network traffic than simple constant or random sources
// - Maintains the IActivePacketSource interface for compatibility
//
// Operation mechanism:
// - The normalProducer generates baseline traffic (typically lower rate, more regular)
// - The burstProducer generates high-intensity traffic (typically higher rate)
// - The MarkovScheduler switches between the two sources based on a Markov process
// - The resulting traffic exhibits periods of normal activity interspersed with bursts
//
// @see ~ActivePacketSource, ~MarkovScheduler
//
module BurstyPacketProducer like IActivePacketSource
{
    parameters:
        @display("i=block/source");
    gates:
        output out;
    submodules:
        normalProducer: ActivePacketSource {
            @display("p=100,100");
        }
        burstProducer: ActivePacketSource {
            @display("p=300,100");
        }
        scheduler: MarkovScheduler {
            @display("p=200,200");
        }
    connections allowunconnected:
        normalProducer.out --> scheduler.in++;
        burstProducer.out --> scheduler.in++;
        scheduler.out --> out;
}
