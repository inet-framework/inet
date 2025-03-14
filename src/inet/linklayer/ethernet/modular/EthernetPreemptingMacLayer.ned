//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.linklayer.ethernet.contract.IEthernetMacLayer;
import inet.protocolelement.contract.IPacketLifeTimer;
import inet.queueing.classifier.PacketClassifier;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketClassifier;
import inet.queueing.contract.IPacketQueue;
import inet.queueing.scheduler.PriorityScheduler;
import inet.queueing.server.PreemptingServer;

//
// Implements IEEE 802.3br Ethernet frame preemption, allowing high-priority
// (express) traffic to interrupt the transmission of low-priority (preemptable)
// traffic. This significantly reduces latency for time-critical applications
// while maintaining bandwidth efficiency for best-effort traffic.
//
// The module separates incoming packets into two paths:
// - Express path: For high-priority traffic that can preempt other transmissions
// - Preemptable path: For lower-priority traffic that can be interrupted
//
// Key components include:
// - Outbound classifier: Separates outgoing packets based on VLAN priority
// - Express MAC layer: Handles high-priority frames without fragmentation
// - Preemptable MAC layer: Processes lower-priority frames with fragmentation support
// - Priority scheduler: Gives preference to express traffic
// - Preempting server: Implements the actual preemption mechanism
// - Inbound classifier: Directs incoming packets to the appropriate processing path
//
// Frame preemption works by allowing the transmission of an express frame to
// interrupt a preemptable frame in progress. The interrupted frame is fragmented,
// and its transmission resumes after the express frame completes. This mechanism
// is transparent to higher layers, which see complete Ethernet frames.
//
// @see ~EthernetFragmentingMacLayer, ~EthernetStreamingMacLayer, ~PreemptingServer
//
module EthernetPreemptingMacLayer like IEthernetMacLayer
{
    parameters:
        volatile double bitrate @unit(bps);
        *.bitrate = default(this.bitrate);
        lifetimer.collectionModule = default("^.queue");
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        lifetimer: <default("")> like IPacketLifeTimer if typename != "" {
            @display("p=100,100;is=s");
        }
        queue: <default("")> like IPacketQueue {
            @display("p=200,100");
        }
        outboundClassifier: <default("PacketClassifier")> like IPacketClassifier {
            classifierClass = default("inet::PacketVlanReqClassifier");
            @display("p=200,200");
        }
        expressMacLayer: <default("EthernetStreamingMacLayer")> like IEthernetMacLayer {
            @display("p=300,300");
        }
        preemptableMacLayer: <default("EthernetFragmentingMacLayer")> like IEthernetMacLayer {
            @display("p=500,300");
        }
        scheduler: PriorityScheduler {
            @display("p=200,400");
        }
        preemptingServer: PreemptingServer {
            datarate = default(parent.bitrate);
            @display("p=200,500");
        }
        inboundClassifier: <default("PacketClassifier")> like IPacketClassifier {
            classifierClass = default("inet::PacketFragmentTagClassifier");
            @display("p=600,400");
        }
        multiplexer: PacketMultiplexer {
            @display("p=600,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> queue.in;
        queue.out --> outboundClassifier.in;
        outboundClassifier.out++ --> expressMacLayer.upperLayerIn;
        outboundClassifier.out++ --> preemptableMacLayer.upperLayerIn;
        expressMacLayer.lowerLayerOut --> scheduler.in++;
        preemptableMacLayer.lowerLayerOut --> scheduler.in++;
        scheduler.out --> preemptingServer.in;
        preemptingServer.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> inboundClassifier.in;
        inboundClassifier.out++ --> expressMacLayer.lowerLayerIn;
        inboundClassifier.out++ --> preemptableMacLayer.lowerLayerIn;
        expressMacLayer.upperLayerOut --> multiplexer.in++;
        preemptableMacLayer.upperLayerOut --> multiplexer.in++;
        multiplexer.out --> { @display("m=n"); } --> upperLayerOut;
}
