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

package inet.protocol.ethernet;

import inet.linklayer.contract.IEthernetInterface;
import inet.networklayer.common.InterfaceEntry;
import inet.protocol.common.InterPacketGap;
import inet.protocol.common.PacketEmitter;
import inet.protocol.common.PacketStreamer;
import inet.protocol.common.PaddingInserter;
import inet.protocol.common.PreemptibleStreamer;
import inet.protocol.common.ProtocolChecker;
import inet.protocol.fragmentation.FragmentTagBasedDefragmenter;
import inet.protocol.ieee8021q.IIeee8021qChecker;
import inet.protocol.ieee8021q.IIeee8021qInserter;
import inet.protocol.ieee8022.IIeee8022LlcChecker;
import inet.protocol.ieee8022.IIeee8022LlcInserter;
import inet.protocol.server.PreemptingServer;
import inet.protocol.transceiver.PreemptibleTransmitter;
import inet.protocol.transceiver.StreamingReceiver;
import inet.protocol.transceiver.contract.IPacketReceiver;
import inet.protocol.transceiver.contract.IPacketTransmitter;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.contract.IPacketServer;
import inet.queueing.contract.ITrafficConditioner;
import inet.queueing.scheduler.PriorityScheduler;

// TODO: add socket handling
// TODO: dispatch for LLC/ethertype
// TODO: connect/disconnect handling
module FlatEthernetInterface extends InterfaceEntry like IEthernetInterface
{
    parameters:
        bool broadcast = default(true);
        bool multicast = default(true);
        string address = default("auto");
        double bitrate @unit(bps);
        string interfaceTableModule;
        @class(InterfaceEntry);
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys;
    submodules:
        ingressConditioner: <default("OmittedTrafficConditioner")> like ITrafficConditioner {
            @display("p=300,100");
        }
        llcInserter: <default("OmittedIeee8022LlcInserter")> like IIeee8022LlcInserter {
            @display("p=300,200");
        }
        typeOrLengthInserter: EthernetTypeOrLengthInserter {
            @display("p=300,300");
        }
        ctagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "c";
            @display("p=300,400");
        }
        stagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "s";
            @display("p=300,500");
        }
        addressInserter: EthernetAddressInserter {
            @display("p=300,600");
        }
        paddingInserter: PaddingInserter {
            minLength = default(60B);
            @display("p=300,700");
        }
        priorityClassifier1: PacketClassifier {
            classifierClass = default("inet::PacketVlanReqClassifier");
            @display("p=300,800");
        }
        expressQueue: <default("PacketQueue")> like IPacketQueue {
            @display("p=100,900");
        }
        normalQueue: <default("PacketQueue")> like IPacketQueue {
            @display("p=500,900");
        }
        priorityScheduler: PriorityScheduler {
            @display("p=300,1300");
        }
        preemptingServer: PreemptingServer {
            datarate = default(bitrate);
            @display("p=300,1400");
        }
        expressStreamer: PacketStreamer {
            @display("p=100,1000");
        }
        expressFcsInserter: EthernetFcsInserter {
            @display("p=100,1100");
        }
        expressPreambleInserter: EthernetPreambleInserter {
            @display("p=100,1200");
        }
        fragmentStreamer: PreemptibleStreamer {
            minPacketLength = 60B;
            @display("p=500,1000");
        }
        fragmentFcsInserter: EthernetFragmentFcsInserter {
            @display("p=500,1100");
        }
        fragmentPreambleInserter: EthernetFragmentPreambleInserter {
            @display("p=500,1200");
        }
        outboundEmitter: PacketEmitter {
            signalName = "packetSentToLower";
            direction = "outbound";
            protocolName = "ethernetphy";
            @display("p=300,1500");
        }
        interFrameGapInserter: InterPacketGap {
            duration = default(96s / dropUnit(bitrate));
            @display("p=300,1600");
        }
        transmitter: <default("PacketTransmitter")> like IPacketTransmitter {
            datarate = default(bitrate);
            @display("p=300,1700");
        }
        protocolChecker: ProtocolChecker {
            @display("p=950,100");
        }
        egressConditioner: <default("OmittedTrafficConditioner")> like ITrafficConditioner {
            @display("p=950,200");
        }
        llcChecker: <default("OmittedIeee8022LlcChecker")> like IIeee8022LlcChecker {
            @display("p=950,300");
        }
        typeOrLengthChecker: EthernetTypeOrLengthChecker {
            @display("p=950,400");
        }
        ctagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "c";
            @display("p=950,500");
        }
        stagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "s";
            @display("p=950,600");
        }
        addressChecker: EthernetAddressChecker {
            @display("p=950,700");
        }
        m2: PacketMultiplexer {
            @display("p=950,1100");
        }
        expressFcsChecker: EthernetFcsChecker {
            @display("p=800,1300");
        }
        expressPreambleChecker: EthernetPreambleChecker {
            @display("p=800,1400");
        }
        defragmenter: FragmentTagBasedDefragmenter {
            @display("p=1100,1200");
        }
        fragmentFcsChecker: EthernetFragmentFcsChecker {
            @display("p=1100,1300");
        }
        fragmentPreambleChecker: EthernetFragmentPreambleChecker {
            @display("p=1100,1400");
        }
        preambleClassifier: PacketClassifier {
            classifierClass = default("inet::PacketEthernetPreambleTypeClassifier");
            @display("p=950,1500");
        }
        inboundEmitter: PacketEmitter {
            signalName = "packetReceivedFromLower";
            direction = "inbound";
            protocolName = "ethernetphy";
            @display("p=950,1600");
        }
        receiver: <default("PacketReceiver")> like IPacketReceiver {
            datarate = default(bitrate);
            @display("p=950,1700");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> ingressConditioner.in;
        ingressConditioner.out --> llcInserter.in;
        llcInserter.out --> typeOrLengthInserter.in;
        typeOrLengthInserter.out --> ctagInserter.in;
        ctagInserter.out --> stagInserter.in;
        stagInserter.out --> addressInserter.in;
        addressInserter.out --> paddingInserter.in;
        paddingInserter.out --> priorityClassifier1.in;
        priorityClassifier1.out++ --> expressQueue.in;
        priorityClassifier1.out++ --> normalQueue.in;
        expressQueue.out --> expressStreamer.in;
        expressStreamer.out --> expressFcsInserter.in;
        expressPreambleInserter.out --> priorityScheduler.in++;
        expressFcsInserter.out --> expressPreambleInserter.in;
        normalQueue.out --> fragmentStreamer.in;
        fragmentStreamer.out --> fragmentFcsInserter.in;
        fragmentPreambleInserter.out --> priorityScheduler.in++;
        fragmentFcsInserter.out --> fragmentPreambleInserter.in;
        priorityScheduler.out --> preemptingServer.in;
        preemptingServer.out --> outboundEmitter.in;
        outboundEmitter.out --> interFrameGapInserter.in;
        interFrameGapInserter.out --> transmitter.in;
        transmitter.out --> phys$o;

        phys$i --> receiver.in;
        receiver.out --> inboundEmitter.in;
        inboundEmitter.out --> preambleClassifier.in;
        preambleClassifier.out++ --> expressPreambleChecker.in;
        preambleClassifier.out++ --> fragmentPreambleChecker.in;
        expressPreambleChecker.out --> expressFcsChecker.in;
        expressFcsChecker.out --> m2.in++;
        fragmentPreambleChecker.out --> fragmentFcsChecker.in;
        fragmentFcsChecker.out --> defragmenter.in;
        defragmenter.out --> m2.in++;
        m2.out --> addressChecker.in;
        addressChecker.out --> stagChecker.in;
        stagChecker.out --> ctagChecker.in;
        ctagChecker.out --> typeOrLengthChecker.in;
        typeOrLengthChecker.out --> llcChecker.in;
        llcChecker.out --> egressConditioner.in;
        egressConditioner.out --> protocolChecker.in;
        protocolChecker.out --> { @display("m=n"); } --> upperLayerOut;
}

moduleinterface ISocketLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

module OmittedSocketLayer like ISocketLayer
{
    parameters:
        @class(OmittedModule);
        @display("i=block/layer,black,50");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    connections:
        upperLayerIn --> lowerLayerOut;
        lowerLayerIn --> upperLayerOut;
}

simple EthernetSocketTable {
    parameters:
	    @display("i=block/table");
}

simple EthernetSocketCommandProcessor {
    parameters:
	    @display("i=block/cogwheel");
    gates:
        input in;
        output out;
}

simple EthernetSocketPacketProcessor {
    parameters:
	    @display("i=block/cogwheel");
    gates:
        input in;
        output out;
}

module EthernetSocketLayer like ISocketLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        socketTable: EthernetSocketTable {
            @display("p=100,100;is=s");
        }
        commandProcessor: EthernetSocketCommandProcessor {
            @display("p=300,100");
        }
        packetProcessor: EthernetSocketPacketProcessor {
            @display("p=500,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> commandProcessor.in;
        commandProcessor.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> packetProcessor.in;
        packetProcessor.out --> { @display("m=n"); } --> upperLayerOut; 
}

moduleinterface ITrafficConditionerLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

module OmittedTrafficConditionerLayer like ITrafficConditionerLayer
{
    parameters:
        @class(OmittedModule);
        @display("i=block/layer,black,50");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    connections:
        upperLayerIn --> lowerLayerOut;
        lowerLayerIn --> upperLayerOut;
}

module TrafficConditionerLayer like ITrafficConditionerLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        ingressConditioner: <default("OmittedTrafficConditioner")> like ITrafficConditioner {
            @display("p=100,100");
        }
        egressConditioner: <default("OmittedTrafficConditioner")> like ITrafficConditioner {
            @display("p=300,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> ingressConditioner.in;
        ingressConditioner.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> egressConditioner.in;
        egressConditioner.out --> { @display("m=n"); } --> upperLayerOut;
}

moduleinterface IIeee8022LlcLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

module OmittedIeee8022LlcLayer like IIeee8022LlcLayer
{
    parameters:
        @class(OmittedModule);
        @display("i=block/layer,black,50");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    connections:
        upperLayerIn --> lowerLayerOut;
        lowerLayerIn --> upperLayerOut;
}

module Ieee8022LlcLayer like IIeee8022LlcLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        llcInserter: <default("Ieee8022LlcInserter")> like IIeee8022LlcInserter {
            @display("p=200,100");
        }
        protocolChecker: ProtocolChecker {
            @display("p=500,100");
        }
        llcChecker: <default("Ieee8022LlcChecker")> like IIeee8022LlcChecker {
            @display("p=500,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> llcInserter.in;
        llcInserter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> llcChecker.in;
        llcChecker.out --> protocolChecker.in;
        protocolChecker.out --> { @display("m=n"); } --> upperLayerOut;
}

moduleinterface IEthernetMacLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

module EthernetMacLayer like IEthernetMacLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        typeOrLengthInserter: EthernetTypeOrLengthInserter {
            @display("p=200,100");
        }
        ctagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "c";
            @display("p=200,200");
        }
        stagInserter: <default("OmittedIeee8021qInserter")> like IIeee8021qInserter {
            vlanTagType = "s";
            @display("p=200,300");
        }
        addressInserter: EthernetAddressInserter {
            @display("p=200,400");
        }
        paddingInserter: PaddingInserter {
            minLength = default(60B);
            @display("p=200,500");
        }
        queue: <default("PacketQueue")> like IPacketQueue {
            @display("p=200,600");
        }
        server: <default("InstantServer")> like IPacketServer {
            @display("p=200,700");
        }
        streamer: <default("OmittedPacketFlow")> like IPacketFlow {
            @display("p=200,800");
        }
        fcsInserter: <default("EthernetFcsInserter")> like IPacketFlow {
            @display("p=200,900");
        }
        preambleInserter: <default("EthernetPreambleInserter")> like IPacketFlow {
            @display("p=200,1000");
        }
        typeOrLengthChecker: EthernetTypeOrLengthChecker {
            @display("p=500,100");
        }
        ctagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "c";
            @display("p=500,200");
        }
        stagChecker: <default("OmittedIeee8021qChecker")> like IIeee8021qChecker {
            vlanTagType = "s";
            @display("p=500,300");
        }
        addressChecker: EthernetAddressChecker {
            @display("p=500,400");
        }
        fcsChecker: <default("EthernetFcsChecker")> like IPacketFilter {
            @display("p=500,900");
        }
        preambleChecker: <default("EthernetPreambleChecker")> like IPacketFilter {
            @display("p=500,1000");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> typeOrLengthInserter.in;
        typeOrLengthInserter.out --> ctagInserter.in;
        ctagInserter.out --> stagInserter.in;
        stagInserter.out --> addressInserter.in;
        addressInserter.out --> paddingInserter.in;
        paddingInserter.out --> queue.in;
        queue.out --> server.in;
        server.out --> streamer.in;
        streamer.out --> fcsInserter.in;
        fcsInserter.out --> preambleInserter.in;
        preambleInserter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> preambleChecker.in;
        preambleChecker.out --> fcsChecker.in;
        fcsChecker.out --> addressChecker.in;
        addressChecker.out --> stagChecker.in;
        stagChecker.out --> ctagChecker.in;
        ctagChecker.out --> typeOrLengthChecker.in;
        typeOrLengthChecker.out --> { @display("m=n"); } --> upperLayerOut;
}

module EthernetStreamingMacLayer extends EthernetMacLayer
{
    parameters:
        server.typename = default("OmittedPacketServer");
        streamer.typename = default("PacketStreamer");
        transmitter.typename = default("StreamingTransmitter");
        receiver.typename = default("StreamingReceiver");
}

module EthernetFragmentingMacLayer extends EthernetMacLayer
{
    parameters:
        server.typename = default("OmittedPacketServer");
        streamer.typename = default("PreemptibleStreamer");
        streamer.minPacketLength = 60B;
        fcsInserter.typename = default("EthernetFragmentFcsInserter");
        preambleInserter.typename = default("EthernetFragmentPreambleInserter");
        transmitter.typename = default("PreemptibleTransmitter");
        fcsChecker.typename = default("EthernetFragmentFcsChecker");
        preambleChecker.typename = default("EthernetFragmentPreambleChecker");
    submodules:
        defragmenter: FragmentTagBasedDefragmenter {
            @display("p=500,800");
        }
    connections:
        fcsChecker.out --> { @reconnect; } --> defragmenter.in;
        defragmenter.out --> { @reconnect; } --> addressChecker.in;
}

module EthernetPreemptingMacLayer like IEthernetMacLayer
{
    parameters:
        volatile double bitrate @unit(bps);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        outboundClassifier: PacketClassifier {
            classifierClass = default("inet::PacketVlanReqClassifier");
            @display("p=200,100");
        }
        expressMacLayer: <default("EthernetStreamingMacLayer")> like IEthernetMacLayer {
            @display("p=300,200");
        }
        normalMacLayer: <default("EthernetFragmentingMacLayer")> like IEthernetMacLayer {
            @display("p=500,200");
        }
        scheduler: PriorityScheduler {
            @display("p=200,300");
        }
        preemptingServer: PreemptingServer {
            datarate = default(bitrate);
            @display("p=200,400");
        }
        inboundClassifier: PacketClassifier {
            classifierClass = default("inet::PacketEthernetPreambleTypeClassifier");
            @display("p=600,300");
        }
        multiplexer: PacketMultiplexer {
            @display("p=600,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> outboundClassifier.in;
        outboundClassifier.out++ --> expressMacLayer.upperLayerIn;
        outboundClassifier.out++ --> normalMacLayer.upperLayerIn;
        expressMacLayer.lowerLayerOut --> scheduler.in++;
        normalMacLayer.lowerLayerOut --> scheduler.in++;
        scheduler.out --> preemptingServer.in;
        preemptingServer.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> inboundClassifier.in;
        inboundClassifier.out++ --> expressMacLayer.lowerLayerIn;
        inboundClassifier.out++ --> normalMacLayer.lowerLayerIn;
        expressMacLayer.upperLayerOut --> multiplexer.in++;
        normalMacLayer.upperLayerOut --> multiplexer.in++;
        multiplexer.out --> { @display("m=n"); } --> upperLayerOut;
}

moduleinterface IEthernetPhyLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

module EthernetPhyLayer like IEthernetPhyLayer
{
    parameters:
        volatile double bitrate @unit(bps);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        outboundEmitter: PacketEmitter {
            signalName = "packetSentToLower";
            direction = "outbound";
            protocolName = "ethernetphy";
            @display("p=150,100");
        }
        interFrameGapInserter: InterPacketGap {
            duration = default(96s / dropUnit(bitrate));
            @display("p=150,200");
        }
        transmitter: PreemptibleTransmitter {
            datarate = default(bitrate);
            @display("p=150,300");
        }
        inboundEmitter: PacketEmitter {
            signalName = "packetReceivedFromLower";
            direction = "inbound";
            protocolName = "ethernetphy";
            @display("p=350,100");
        }
        receiver: StreamingReceiver {
            datarate = default(bitrate);
            @display("p=350,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> outboundEmitter.in;
        outboundEmitter.out --> interFrameGapInserter.in;
        interFrameGapInserter.out --> transmitter.in;
        transmitter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> receiver.in;
        receiver.out --> inboundEmitter.in;
        inboundEmitter.out --> { @display("m=n"); } --> upperLayerOut;
}

module LayeredEthernetInterface extends InterfaceEntry like IEthernetInterface
{
    parameters:
        bool broadcast = default(true);
        bool multicast = default(true);
        string address = default("auto");
        double bitrate @unit(bps);
        string interfaceTableModule;
        *.bitrate = default(bitrate);
        @class(InterfaceEntry);
        @display("i=block/ifcard;bgb=600,");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys;
    submodules:
        socketLayer: <default("OmittedSocketLayer")> like ISocketLayer {
            @display("p=400,100");
        }
        trafficConditionerLayer: <default("OmittedTrafficConditionerLayer")> like ITrafficConditionerLayer {
            @display("p=400,200");
        }
        llcLayer: <default("OmittedIeee8022LlcLayer")> like IIeee8022LlcLayer {
            @display("p=400,300");
        }
        macLayer: <default("EthernetMacLayer")> like IEthernetMacLayer {
            @display("p=400,400");
        }
        phyLayer: <default("EthernetPhyLayer")> like IEthernetPhyLayer {
            @display("p=400,500");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> socketLayer.upperLayerIn;
        socketLayer.lowerLayerOut --> trafficConditionerLayer.upperLayerIn;
        trafficConditionerLayer.lowerLayerOut --> llcLayer.upperLayerIn;
        llcLayer.lowerLayerOut --> macLayer.upperLayerIn;
        macLayer.lowerLayerOut --> phyLayer.upperLayerIn;
        phyLayer.lowerLayerOut --> { @display("m=s"); } --> phys$o;

        phys$i --> { @display("m=s"); } --> phyLayer.lowerLayerIn;
        phyLayer.upperLayerOut --> macLayer.lowerLayerIn;
        macLayer.upperLayerOut --> llcLayer.lowerLayerIn;
        llcLayer.upperLayerOut --> trafficConditionerLayer.lowerLayerIn;
        trafficConditionerLayer.upperLayerOut --> socketLayer.lowerLayerIn;
        socketLayer.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
}
