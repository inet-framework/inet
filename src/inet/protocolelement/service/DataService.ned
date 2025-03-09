//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.service;

import inet.protocolelement.aggregation.SubpacketLengthHeaderBasedAggregator;
import inet.protocolelement.aggregation.SubpacketLengthHeaderBasedDeaggregator;
import inet.protocolelement.fragmentation.FragmentNumberHeaderBasedDefragmenter;
import inet.protocolelement.ordering.Reordering;
import inet.protocolelement.ordering.SequenceNumbering;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;

//
// Provides comprehensive data processing services for packet-based communication.
// Combines multiple protocol elements to handle aggregation, fragmentation,
// sequencing, and queuing in both transmission and reception paths.
//
// Operation:
// - Transmission path (upper to lower):
//   * Aggregates small packets into larger ones
//   * Fragments large packets if needed
//   * Adds sequence numbers for reliable delivery
//   * Queues packets for transmission
//   * Serves packets according to configured policy
// - Reception path (lower to upper):
//   * Reorders packets based on sequence numbers
//   * Reassembles fragmented packets
//   * Extracts original packets from aggregated ones
//
// Particularly useful for:
// - Implementing reliable data transfer services
// - Building protocol stacks with comprehensive data handling
// - Supporting efficient bandwidth utilization
// - Ensuring ordered packet delivery
// - Creating custom transport layer implementations
//
// @see ~ForwardingService, ~InterfaceService, ~MacService, ~SelectivityService
//
module DataService
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        aggregator: SubpacketLengthHeaderBasedAggregator {
            @display("p=150,100");
        }
        fragmenter: FragmentNumberHeaderBasedDefragmenter {
            @display("p=150,200");
        }
        sequenceNumbering: SequenceNumbering {
            @display("p=150,300");
        }
        queue: <default("DropTailQueue")> like IPacketQueue {
            @display("p=150,400");
        }
        server: <default("PacketServer")> like IPacketServer {
            @display("p=150,500");
        }
        deaggregator: SubpacketLengthHeaderBasedDeaggregator {
            @display("p=450,100");
        }
        defragmenter: FragmentNumberHeaderBasedDefragmenter {
            @display("p=450,200");
        }
        reordering: Reordering {
            @display("p=450,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> aggregator.in;
        aggregator.out --> fragmenter.in;
        fragmenter.out --> sequenceNumbering.in;
        sequenceNumbering.out --> queue.in;
        queue.out --> server.in;
        server.out --> { @display("m=s"); } --> lowerLayerOut;
        lowerLayerIn --> { @display("m=s"); } --> reordering.in;
        reordering.out --> defragmenter.in;
        defragmenter.out --> deaggregator.in;
        deaggregator.out --> { @display("m=n"); } --> upperLayerOut;
}
