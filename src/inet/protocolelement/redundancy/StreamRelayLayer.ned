//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.redundancy;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketFilter;
import inet.queueing.contract.IPacketPusher;

//
// Combines stream merging and splitting functionality into a single protocol layer.
// Provides bidirectional redundancy capabilities for implementing fault-tolerant
// communication systems.
//
// Operation:
// - Processes incoming packets through the stream merger
// - Processes outgoing packets through the stream splitter
// - Eliminates duplicate packets from redundant streams
// - Creates redundant packet copies for multiple transmission paths
// - Maintains separate processing paths for upward and downward packet flows
//
// @see ~StreamMerger, ~StreamSplitter, ~StreamCoderLayer
//
module StreamRelayLayer like IProtocolLayer
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
        merger: <default("StreamMerger")> like IPacketFilter if typename != "" {
            @display("p=200,150;i=block/join");
        }
        splitter: <default("StreamSplitter")> like IPacketPusher if typename != "" {
            @display("p=500,150;i=block/fork");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> splitter.in;
        splitter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> merger.in;
        merger.out --> { @display("m=n"); } --> upperLayerOut;
}
