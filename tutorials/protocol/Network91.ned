//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.tutorials.protocol;

import inet.common.MessageDispatcher;
import inet.networklayer.common.NetworkInterface;
import inet.networklayer.common.InterfaceTable;
import inet.protocolelement.acknowledgement.ReceiveWithAcknowledge;
import inet.protocolelement.acknowledgement.Resending;
import inet.protocolelement.acknowledgement.SendWithAcknowledge;
import inet.protocolelement.aggregation.SubpacketLengthHeaderBasedAggregator;
import inet.protocolelement.aggregation.SubpacketLengthHeaderBasedDeaggregator;
import inet.protocolelement.checksum.FcsHeaderChecker;
import inet.protocolelement.checksum.FcsHeaderInserter;
import inet.protocolelement.dispatching.ReceiveWithProtocol;
import inet.protocolelement.dispatching.SendWithProtocol;
import inet.protocolelement.forwarding.Forwarding;
import inet.protocolelement.forwarding.ReceiveWithHopLimit;
import inet.protocolelement.forwarding.SendWithHopLimit;
import inet.protocolelement.fragmentation.FragmentNumberHeaderBasedDefragmenter;
import inet.protocolelement.ordering.Reordering;
import inet.protocolelement.ordering.SequenceNumbering;
import inet.protocolelement.selectivity.ReceiveAtL3Address;
import inet.protocolelement.selectivity.ReceiveAtMacAddress;
import inet.protocolelement.selectivity.ReceiveAtPort;
import inet.protocolelement.selectivity.SendToL3Address;
import inet.protocolelement.selectivity.SendToMacAddress;
import inet.protocolelement.selectivity.SendToPort;
import inet.protocolelement.transceiver.PacketTransmitter;
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

module Interface extends NetworkInterface
{
    parameters:
        string interfaceTableModule;
        @class(NetworkInterface);
        @display("i=block/ifcard");
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
        inout phys;
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
        fcsInserter: FcsHeaderInserter {
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
        fcsChecker: FcsHeaderChecker {
            @display("p=450,500");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> sendToMacAddress.in;
        sendToMacAddress.out --> resending.in;
        resending.out --> sendWithAcknowledge.in;
        sendWithAcknowledge.out --> m1.in++;
        m1.out --> sendWithProtocol.in;
        sendWithProtocol.out --> fcsInserter.in;
        fcsInserter.out --> transmitter.in;
        transmitter.out --> { @display("m=s"); } --> phys$o;
        phys$i --> { @display("m=s"); } --> fcsChecker.in;
        fcsChecker.out --> receiveWithProtocol.in;
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
            g[i] <--> { @display("m=s"); } <--> interface[i].phys;
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
