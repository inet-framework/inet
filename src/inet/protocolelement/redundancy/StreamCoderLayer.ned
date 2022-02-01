//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFlow;

//
// This module combines a stream decoder and a stream encoder into a layer. For
// incoming packets the stream decoder determines the stream of the packet. For
// outgoing packets the stream encoder determines the VLAN id, etc. of the packet.
//
module StreamCoderLayer like IProtocolLayer
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(this.interfaceTableModule);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        decoder: <default("StreamDecoder")> like IPacketFlow if typename != "" {
            @display("p=200,150;i=block/blackboard");
        }
        encoder: <default("StreamEncoder")> like IPacketFlow if typename != "" {
            @display("p=500,150;i=block/blackboard");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> encoder.in;
        encoder.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> decoder.in;
        decoder.out --> { @display("m=n"); } --> upperLayerOut;
}
