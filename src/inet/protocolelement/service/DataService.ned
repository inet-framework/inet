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
