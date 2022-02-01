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
// This module combines a stream merger and a stream splitter module into a
// stream relay layer. For incoming packets the identified streams are merged
// by the stream merger. For outgoing packets the requested streams are split
// by the stream splitter.
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
