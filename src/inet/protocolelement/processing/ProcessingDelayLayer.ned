//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.protocolelement.processing;

import inet.queueing.contract.IPacketDelayer;

//
// Implements a processing delay layer that models packet processing time in
// network devices. Contains separate delayer modules for ingress and egress
// packet flows to simulate realistic processing delays.
//
module ProcessingDelayLayer like IProcessingDelayLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        ingress: <default("PacketDelayer")> like IPacketDelayer {
            parameters:
                @display("p=150,100");
        }
        egress: <default("PacketDelayer")> like IPacketDelayer {
            parameters:
                @display("p=350,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> egress.in;
        egress.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> ingress.in;
        ingress.out --> { @display("m=n"); } --> upperLayerOut;
}
