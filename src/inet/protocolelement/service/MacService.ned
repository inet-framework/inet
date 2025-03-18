//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.service;

import inet.protocolelement.fragmentation.FragmentNumberHeaderBasedDefragmenter;
import inet.queueing.classifier.DynamicClassifier;
import inet.queueing.common.PacketMultiplexer;

//
// Provides packet defragmentation services for a specific peer. Uses a dynamic
// classifier to route packets to the appropriate defragmenter based on packet
// attributes, then multiplexes the processed packets to a single output.
//
module PeerService
{
    parameters:
        int numDefragmenter = default(0);
        defragmenter[*].deleteSelf = true;
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        multiplexer: PacketMultiplexer {
            @display("p=150,100");
        }
        defragmenter[numDefragmenter]: FragmentNumberHeaderBasedDefragmenter {
            @display("p=150,200,row,100");
        }
        classifier: DynamicClassifier {
            @display("p=150,300");
        }
    connections:
        in --> classifier.in;
        for i=0..numDefragmenter-1 {
            classifier.out++ --> multiplexer.in++;
        }
        multiplexer.out --> out;
}

//
// Manages multiple peer services for MAC-level packet processing. Uses a
// dynamic classifier to route packets to the appropriate peer service based on
// packet attributes, then multiplexes the processed packets to a single output.
//
module MacService
{
    parameters:
        int numPeers = default(0);
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        multiplexer: PacketMultiplexer {
            @display("p=150,100");
        }
        peer[numPeers]: PeerService {
            @display("p=150,200,row,100");
        }
        classifier: DynamicClassifier {
            @display("p=150,300");
        }
    connections:
        in --> classifier.in;
        for i=0..numPeers - 1 {
            classifier.out++ --> multiplexer.in++;
        }
        multiplexer.out --> out;
}
