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

//
// Implements the Ethernet MAC (Medium Access Control) layer functionality using
// a modular architecture that separates different protocol aspects into distinct
// components. This design provides flexibility while maintaining standard Ethernet
// MAC behavior.
//
// The MAC layer handles several key functions:
// - Packet queuing and scheduling for transmission
// - Frame Check Sequence (FCS) verification for received frames
// - Packet lifetime management for performance monitoring
// - Packet flow statistics collection and reporting
//
// The module consists of the following components:
// - Lifetime timer: Tracks packet processing time through the module
// - Queue: Buffers packets waiting for transmission
// - Server: Schedules packet processing based on configured parameters
// - Outbound emitter: Processes and signals outgoing packets
// - FCS checker: Verifies frame integrity for incoming packets
// - Inbound emitter: Processes and signals incoming packets
//
// This implementation follows a pipeline architecture where packets flow through
// the components in sequence, with each component performing a specific function.
// The modular design allows for easy customization by replacing individual components
// with specialized implementations.
//
// @see ~IEthernetMacLayer, ~LayeredEthernetInterface
//
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
            serveSchedulingPriority = default(0);
            @display("p=200,200");
        }
        outboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=200,400");
        }
        fcsChecker: <default("EthernetFcsChecker")> like IPacketFilter {
            popFcs = default(false);
            @display("p=500,300");
        }
        inboundEmitter: <default("PacketEmitter")> like IPacketFlow {
            @display("p=500,400");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> queue.in;
        queue.out --> server.in;
        server.out --> outboundEmitter.in;
        outboundEmitter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> inboundEmitter.in;
        inboundEmitter.out --> fcsChecker.in;
        fcsChecker.out --> { @display("m=n"); } --> upperLayerOut;
}
