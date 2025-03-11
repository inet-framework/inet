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
// Applications:
// - Modeling realistic Internet traffic with self-similar properties
// - Simulating bursty application traffic (e.g., video streaming, web browsing)
// - Testing network equipment under variable load conditions
// - Evaluating congestion control algorithms
// - Benchmarking buffer sizing in network devices
//
// The BurstyPacketProducer is particularly useful in scenarios where:
// - Traffic patterns exhibit on-off behavior
// - Network performance under variable load needs to be evaluated
// - More realistic traffic models are required than simple Poisson processes
// - Self-similar or long-range dependent traffic needs to be generated
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
