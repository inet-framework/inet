//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.common;

import inet.common.MessageDispatcher;
import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.common.PacketMultiplexer;
import inet.queueing.contract.IPacketDelayer;
import inet.queueing.contract.IPacketFlow;

//
// Protocol layer that handles packet direction reversal for forwarding in Ethernet networks. Converts 
// incoming packets to outgoing packets by reversing their direction and preserving necessary tags and 
// attributes. Supports packet forwarding by transforming inbound packets to outbound packets while 
// maintaining addressing, VLAN, priority, and other relevant information.
//
// @see ~PacketDirectionReverser, ~IProtocolLayer, ~MessageDispatcher
//
module DirectionReverserLayer like IProtocolLayer
{
    parameters:
        string interfaceTableModule; // Relative module path of the interface table
        *.interfaceTableModule = default(this.interfaceTableModule);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        cutthroughBarrier: <default("")> like IPacketDelayer {
            @display("p=100,150");
        }
        fork: MessageDispatcher {
            forwardServiceRegistration = false;
            @display("p=100,300;i=-;b=10,10,oval,grey,,1");
        }
        delayer: <default("")> like IPacketDelayer {
            @display("p=300,300");
        }
        reverser: <default("PacketDirectionReverser")> like IPacketFlow {
            @display("p=500,300");
        }
        join: PacketMultiplexer {
            forwardProtocolRegistration = false;
            @display("p=700,300");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> join.in++;
        join.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> fork.in++;
        fork.out++ --> cutthroughBarrier.in;
        cutthroughBarrier.out --> { @display("m=n"); } --> upperLayerOut;

        fork.out++ --> delayer.in;
        delayer.out --> reverser.in;
        reverser.out --> join.in++;
}
