//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.service;

import inet.protocolelement.fragmentation.FragmentNumberHeaderBasedDefragmenter;
import inet.queueing.classifier.DynamicClassifier;
import inet.queueing.common.PacketMultiplexer;

//
// Provides packet processing services for communication with a single peer.
// Handles packet defragmentation and classification for peer-to-peer communication.
//
// Operation:
// - Classifies incoming packets based on their properties
// - Reassembles fragmented packets from the peer
// - Multiplexes packets from different processing paths
// - Supports dynamic creation and deletion of defragmenters
//
// @see ~MacService
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
// Provides MAC layer services for communication with multiple peers.
// Manages a collection of peer services and handles packet classification
// and multiplexing between them.
//
// Operation:
// - Classifies incoming packets to determine the appropriate peer
// - Routes packets to the corresponding peer service
// - Multiplexes packets from different peers into a single output
// - Supports dynamic peer configuration
//
// @see ~PeerService, ~DataService, ~ForwardingService, ~InterfaceService, ~SelectivityService
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
