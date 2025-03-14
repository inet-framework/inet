//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.linklayer.ethernet.contract.IEthernetMacLayer;
import inet.protocolelement.contract.IPacketLifeTimer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketFlow;
import inet.queueing.contract.IPacketQueue;

//
// Implements a streaming Ethernet MAC layer that processes packets as continuous
// streams rather than discrete units. Provides standard Ethernet MAC functionality
// with a simplified processing pipeline optimized for continuous transmission.
//
// The module processes outgoing packets through a pipeline that:
// - Queues packets waiting for transmission
// - Streams packets to the lower layer with minimum packet length enforcement
// - Emits signals for outbound packet statistics
//
// For incoming packets, the module:
// - Verifies FCS integrity
// - Emits signals for inbound packet statistics
//
// This implementation is typically used as the express traffic path in the
// EthernetPreemptingMacLayer, where it handles high-priority traffic that
// can preempt lower-priority traffic. Unlike the EthernetFragmentingMacLayer,
// it does not support packet fragmentation.
//
// @see ~EthernetPreemptingMacLayer, ~EthernetFragmentingMacLayer, ~EthernetMacLayer
//
module EthernetStreamingMacLayer like IEthernetMacLayer
{
    parameters:
        double bitrate @unit(bps);
        *.bitrate = default(this.bitrate);
        lifetimer.collectionModule = default("^.queue");
        streamer.minPacketLength = 60B;
        outboundEmitter.signalName = "packetSentToLower";
        outboundEmitter.direction = "outbound";
        inboundEmitter.signalName = "packetReceivedFromLower";
        inboundEmitter.direction = "inbound";
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
        queue: <default("PacketQueue")> like IPacketQueue {
            @display("p=200,100");
        }
        streamer: <default("PacketStreamer")> like IPacketFlow {
            @display("p=200,200");
        }
        outboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=200,300");
        }
        fcsChecker: <default("EthernetFcsChecker")> like IPacketFilter {
            popFcs = default(false);
            @display("p=500,200");
        }
        inboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=500,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> queue.in;
        queue.out --> streamer.in;
        streamer.out --> outboundEmitter.in;
        outboundEmitter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> inboundEmitter.in;
        inboundEmitter.out --> fcsChecker.in;
        fcsChecker.out --> { @display("m=n"); } --> upperLayerOut;
}
