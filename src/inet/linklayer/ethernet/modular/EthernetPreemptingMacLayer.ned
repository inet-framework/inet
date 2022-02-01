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
import inet.queueing.contract.IPacketQueue;
import inet.queueing.scheduler.PriorityScheduler;
import inet.queueing.server.PreemptingServer;

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
        outboundClassifier: PacketClassifier {
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
        inboundClassifier: PacketClassifier {
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
