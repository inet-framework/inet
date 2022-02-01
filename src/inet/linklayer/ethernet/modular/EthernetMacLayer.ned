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
import inet.queueing.contract.IPacketServer;

module EthernetMacLayer like IEthernetMacLayer
{
    parameters:
        double bitrate @unit(bps);
        *.bitrate = default(this.bitrate);
        lifetimer.collectionModule = default("^.queue");
        outboundEmitter.signalName = "packetSentToLower";
        outboundEmitter.direction = "outbound";
        inboundEmitter.signalName = "packetReceivedFromLower";
        inboundEmitter.direction = "inbound";
        @display("i=block/layer");
        @class(EthernetMacLayer);
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
        server: <default("InstantServer")> like IPacketServer {
            @display("p=200,200");
        }
        streamer: <default("")> like IPacketFlow {
            @display("p=200,300");
        }
        fcsInserter: <default("EthernetFcsInserter")> like IPacketFlow {
            @display("p=200,400");
        }
        outboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=200,500");
        }
        fcsChecker: <default("EthernetFcsChecker")> like IPacketFilter {
            @display("p=500,400");
        }
        inboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=500,500");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> queue.in;
        queue.out --> server.in;
        server.out --> streamer.in;
        streamer.out --> fcsInserter.in;
        fcsInserter.out --> outboundEmitter.in;
        outboundEmitter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> inboundEmitter.in;
        inboundEmitter.out --> fcsChecker.in;
        fcsChecker.out --> { @display("m=n"); } --> upperLayerOut;
}
