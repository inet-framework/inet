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

import inet.common.MessageDispatcher;
import inet.networklayer.common.InterfaceEntry;
import inet.networklayer.common.InterfaceTable;
import inet.protocol.acknowledgement.ReceiveWithAcknowledge;
import inet.protocol.acknowledgement.Resending;
import inet.protocol.acknowledgement.SendWithAcknowledge;
import inet.protocol.aggregation.SubpacketLengthHeaderBasedAggregator;
import inet.protocol.aggregation.SubpacketLengthHeaderBasedDeaggregator;
import inet.protocol.checksum.CrcHeaderChecker;
import inet.protocol.checksum.CrcHeaderInserter;
import inet.protocol.dispatching.ReceiveWithProtocol;
import inet.protocol.dispatching.SendWithProtocol;
import inet.protocol.forwarding.Forwarding;
import inet.protocol.forwarding.ReceiveWithHopLimit;
import inet.protocol.forwarding.SendWithHopLimit;
import inet.protocol.fragmentation.FragmentNumberHeaderBasedDefragmenter;
import inet.protocol.ordering.Reordering;
import inet.protocol.ordering.SequenceNumbering;
import inet.protocol.selectivity.ReceiveAtL3Address;
import inet.protocol.selectivity.ReceiveAtMacAddress;
import inet.protocol.selectivity.ReceiveAtPort;
import inet.protocol.selectivity.SendToL3Address;
import inet.protocol.selectivity.SendToMacAddress;
import inet.protocol.selectivity.SendToPort;
import inet.protocol.transceiver.PacketTransmitter;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IActivePacketSource;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;
import inet.queueing.contract.IPassivePacketSink;


moduleinterface IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input in;
        output out;
}

module ClientApp like IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input in;
        output out;
    submodules:
        source: <default("ActivePacketSource")> like IActivePacketSource {
            @display("p=150,100");
        }
        sendToPort: SendToPort {
            @display("p=150,200");
        }
        sendToL3Address: SendToL3Address {
            @display("p=150,300");
        }
    connections allowunconnected:
        source.out --> sendToPort.in;
        sendToPort.out --> sendToL3Address.in;
        sendToL3Address.out --> { @display("m=s"); } --> out;
}

module ServerApp like IApp
{
    parameters:
        @display("i=block/app");
    gates:
        input in;
        output out;
    submodules:
        sink: <default("PassivePacketSink")> like IPassivePacketSink {
            @display("p=150,100");
        }
        receiveAtPort: ReceiveAtPort {
            @display("p=150,200");
        }
        receiveAtL3Address: ReceiveAtL3Address {
            @display("p=150,300");
        }
    connections allowunconnected:
        in --> { @display("m=s"); } --> receiveAtL3Address.in;
        receiveAtL3Address.out --> receiveAtPort.in;
        receiveAtPort.out --> sink.in;
}

module ForwardingService
{
    parameters:
        @display("i=block/routing");
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        d1: MessageDispatcher {
            @display("p=300,100");
        }
        forwarding: Forwarding {
            @display("p=300,200");
        }
        sendWithHopLimit: SendWithHopLimit {
            @display("p=150,300");
        }
        receiveWithHopLimit: ReceiveWithHopLimit {
            @display("p=450,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> d1.in++;
        d1.out++ --> forwarding.in;
        forwarding.out --> d1.in++;
        d1.out++ --> sendWithHopLimit.in;
        sendWithHopLimit.out --> { @display("m=s"); } --> lowerLayerOut;
        lowerLayerIn --> { @display("m=s"); } --> receiveWithHopLimit.in;
        receiveWithHopLimit.out --> d1.in++;
        d1.out++ --> { @display("m=n"); } --> upperLayerOut;
}

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

module Interface extends InterfaceEntry
{
    parameters:
        string interfaceTableModule;
        @class(inet::InterfaceEntry);
        @display("i=block/ifcard");
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
        inout g;
    submodules:
        sendToMacAddress: SendToMacAddress {
            @display("p=150,100");
        }
        resending: Resending {
            @display("p=150,200");
        }
        sendWithAcknowledge: SendWithAcknowledge {
            @display("p=150,300");
        }
        m1: PacketMultiplexer {
            @display("p=150,400");
        }
        sendWithProtocol: SendWithProtocol {
            @display("p=150,500");
        }
        crcInserter: CrcHeaderInserter {
            @display("p=150,600");
        }
        transmitter: PacketTransmitter {
            @display("p=150,700");
        }
        receiveAtMacAddress: ReceiveAtMacAddress {
            @display("p=450,100");
        }
        receiveWithAcknowledge: ReceiveWithAcknowledge {
            @display("p=450,200");
        }
        d1: MessageDispatcher {
            @display("p=450,300");
        }
        receiveWithProtocol: ReceiveWithProtocol {
            @display("p=450,400");
        }
        crcChecker: CrcHeaderChecker {
            @display("p=450,500");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> sendToMacAddress.in;
        sendToMacAddress.out --> resending.in;
        resending.out --> sendWithAcknowledge.in;
        sendWithAcknowledge.out --> m1.in++;
        m1.out --> sendWithProtocol.in;
        sendWithProtocol.out --> crcInserter.in;
        crcInserter.out --> transmitter.in;
        transmitter.out --> { @display("m=s"); } --> g$o;
        g$i --> { @display("m=s"); } --> crcChecker.in;
        crcChecker.out --> receiveWithProtocol.in;
        receiveWithProtocol.out --> d1.in++;
        d1.out++ --> receiveWithAcknowledge.in;
        receiveWithAcknowledge.out --> receiveAtMacAddress.in;
        receiveWithAcknowledge.ackOut --> m1.in++;
        d1.out++ --> sendWithAcknowledge.ackIn;
        receiveAtMacAddress.out --> { @display("m=n"); } --> upperLayerOut;
}

module Node
{
    parameters:
        int numInterfaces = default(0);
        interface[*].interfaceTableModule = "^.interfaceTable";
        @networkNode;
    gates:
        inout g[numInterfaces];
    submodules:
        interfaceTable: InterfaceTable {
            @display("p=150,100");
        }
        app: <default("")> like IApp if typename != "" {
            @display("p=450,100");
        }
        forwardingService: ForwardingService {
            @display("p=450,200");
        }
        dataService: DataService {
            @display("p=450,300");
        }
        d1: MessageDispatcher {
            @display("p=450,400");
        }
        interface[sizeof(g)]: Interface {
            @display("p=450,500,row,150");
        }
    connections:
        app.out --> forwardingService.upperLayerIn if exists(app);
        forwardingService.upperLayerOut --> app.in if exists(app);
        forwardingService.lowerLayerOut --> dataService.upperLayerIn;
        dataService.lowerLayerOut --> d1.in++;
        d1.out++ --> dataService.lowerLayerIn;
        dataService.upperLayerOut --> forwardingService.lowerLayerIn;
        for i=0..sizeof(interface)-1 {
            interface[i].upperLayerOut --> d1.in++;
            d1.out++ --> interface[i].upperLayerIn;
            g[i] <--> { @display("m=s"); } <--> interface[i].g;
        }
}

module Host extends Node
{
    parameters:
        @display("i=device/pc");
}

network Router extends Node
{
    parameters:
        @display("i=device/router");
}

network Network91
{
    submodules:
        client: Host {
            @display("p=100,100");
        }
        router1: Router {
            @display("p=300,100");
        }
        router2: Router {
            @display("p=500,100");
        }
        router3: Router {
            @display("p=400,300");
        }
        server: Host {
            @display("p=700,300");
        }
    connections:
        client.g++ <--> {  delay = 1ms; } <--> router1.g++;
        router1.g++ <--> {  delay = 1ms; } <--> router2.g++;
        router1.g++ <--> {  delay = 1ms; } <--> router3.g++;
        router2.g++ <--> {  delay = 1ms; } <--> router3.g++;
        router3.g++ <--> {  delay = 1ms; } <--> server.g++;
}
