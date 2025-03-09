//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFlow;

//
// Combines stream encoding and decoding functionality into a single protocol layer.
// Provides bidirectional stream identification and mapping capabilities for
// implementing redundant communication systems.
//
// Operation:
// - Processes incoming packets through the stream decoder
// - Processes outgoing packets through the stream encoder
// - Maps network parameters (interface, addresses, VLAN) to stream identifiers
// - Maps stream identifiers to network parameters
// - Maintains separate processing paths for upward and downward packet flows
//
// @see ~StreamEncoder, ~StreamDecoder, ~StreamRelayLayer
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
