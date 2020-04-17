//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

package inet.tutorials.protocol;

import inet.protocol.checksum.FcsHeaderChecker;
import inet.protocol.checksum.FcsHeaderInserter;
import inet.protocol.classifier.DynamicClassifier;
import inet.protocol.common.InterPacketGap;
import inet.protocol.fragmentation.FragmentNumberHeaderBasedDefragmenter;
import inet.protocol.ordering.SequenceNumbering;
import inet.protocol.queue.InProgressQueue;
import inet.protocol.server.PreemptingServer;
import inet.protocol.transceiver.PacketReceiver;
import inet.protocol.transceiver.PreemptibleTransmitter;
import inet.queueing.classifier.ContentBasedClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.marker.PacketTagger;
import inet.queueing.queue.PacketQueue;
import inet.queueing.server.InstantServer;
import inet.queueing.sink.PassivePacketSink;
import inet.queueing.source.ActivePacketSource;
import ned.TransmissionChannel;


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
        interPacketGap: InterPacketGap {
            @display("p=200,1200");
        }
        transmitter: PreemptibleTransmitter {
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
        preemptingServer.preemptedOut --> multiplexer2.in++;
        preemptingServer.out --> fcsInserter.in;
        fcsInserter.out --> interPacketGap.in;
        interPacketGap.out --> transmitter.in;
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
        sender.g --> TransmissionChannel {  delay = 1us; } --> receiver.g;
}
