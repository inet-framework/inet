//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.tutorials.protocol;

import inet.protocolelement.checksum.FcsHeaderChecker;
import inet.protocolelement.checksum.FcsHeaderInserter;
import inet.protocolelement.common.InterpacketGapInserter;
import inet.protocolelement.fragmentation.FragmentNumberHeaderBasedDefragmenter;
import inet.protocolelement.ordering.SequenceNumbering;
import inet.queueing.queue.InProgressQueue;
import inet.protocolelement.transceiver.PacketReceiver;
import inet.protocolelement.transceiver.StreamThroughTransmitter;
import inet.queueing.classifier.ContentBasedClassifier;
import inet.queueing.classifier.DynamicClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.marker.PacketTagger;
import inet.queueing.queue.PacketQueue;
import inet.queueing.server.InstantServer;
import inet.queueing.server.PreemptingServer;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;

module SenderHost
{
    parameters:
        @networkNode;
        @display("i=device/pc");
    gates:
        output g;
    submodules:
        sourceA: ActivePacketSource {
            @display("p=100,100");
        }
        taggerA: PacketTagger {
            @display("p=100,200");
        }
        sourceB: ActivePacketSource {
            @display("p=200,100");
        }
        taggerB: PacketTagger {
            @display("p=200,200");
        }
        sourceC: ActivePacketSource {
            @display("p=300,100");
        }
        taggerC: PacketTagger {
            @display("p=300,200");
        }
        multiplexer1: PacketMultiplexer {
            @display("p=200,300");
        }
        sequenceNumbering: SequenceNumbering {
            @display("p=200,400");
        }
        pendingQueue: PacketQueue {
            @display("p=200,500");
        }
        server: InstantServer {
            @display("p=200,600");
        }
        fragmenter: FragmentNumberHeaderBasedDefragmenter {
            @display("p=200,700");
        }
        multiplexer2: PacketMultiplexer {
            @display("p=250,800");
        }
        inProgressQueue: InProgressQueue {
            @display("p=150,900");
        }
        preemptingServer: PreemptingServer {
            @display("p=250,1000");
        }
        fcsInserter: FcsHeaderInserter {
            @display("p=200,1100");
        }
        InterpacketGapInserter: InterpacketGapInserter {
            @display("p=200,1200");
        }
        transmitter: StreamThroughTransmitter {
            @display("p=200,1300");
        }
    connections:
        sourceA.out --> taggerA.in;
        sourceB.out --> taggerB.in;
        sourceC.out --> taggerC.in;
        taggerA.out --> multiplexer1.in++;
        taggerB.out --> multiplexer1.in++;
        taggerC.out --> multiplexer1.in++;
        multiplexer1.out --> sequenceNumbering.in;
        sequenceNumbering.out --> pendingQueue.in;
        pendingQueue.out --> server.in;
        server.out --> fragmenter.in;
        fragmenter.out --> multiplexer2.in++;
        multiplexer2.out --> inProgressQueue.in;
        inProgressQueue.out --> preemptingServer.in;
        // TODO:        preemptingServer.preemptedOut --> multiplexer2.in++;
        preemptingServer.out --> fcsInserter.in;
        fcsInserter.out --> InterpacketGapInserter.in;
        InterpacketGapInserter.out --> transmitter.in;
        transmitter.out --> g;
}

module DynamicDefragmenter
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

module ReceiverHost
{
    parameters:
        @networkNode;
        @display("i=device/pc");
    gates:
        input g;
    submodules:
        sinkA: PassivePacketSink {
            @display("p=100,100");
        }
        sinkB: PassivePacketSink {
            @display("p=200,100");
        }
        sinkC: PassivePacketSink {
            @display("p=300,100");
        }
        classifier: ContentBasedClassifier {
            @display("p=200,200");
        }
        defragmenter: DynamicDefragmenter {
            @display("p=200,300");
        }
        fcsChecker: FcsHeaderChecker {
            @display("p=200,400");
        }
        receiver: PacketReceiver {
            @display("p=200,500");
        }
    connections:
        g --> receiver.in;
        receiver.out --> fcsChecker.in;
        fcsChecker.out --> defragmenter.in;
        defragmenter.out --> classifier.in;
        classifier.out++ --> sinkA.in;
        classifier.out++ --> sinkB.in;
        classifier.out++ --> sinkC.in;
}

network Network90
{
    submodules:
        sender: SenderHost {
            @display("p=100,100");
        }
        receiver: ReceiverHost {
            @display("p=200,100");
        }
    connections:
        sender.g --> {  delay = 1us; } --> receiver.g;
}
