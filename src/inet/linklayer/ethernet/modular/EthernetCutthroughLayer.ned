//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

//
// Implements Ethernet cut-through switching functionality by combining source and sink
// components. Provides a layer that enables packets to begin transmission before their
// reception is complete, significantly reducing end-to-end delay in Ethernet networks.
// Connects to upper and lower layers, passing packets through the appropriate cut-through
// components based on direction. Requires a MAC table for forwarding decisions and
// configures all submodules with the specified bitrate.
//
// @see EthernetCutthroughBarrier, EthernetCutthroughSource, EthernetCutthroughSink, EthernetCutthroughInterface
//
module EthernetCutthroughLayer
{
    parameters:
        string macTableModule;
        double bitrate @unit(bps);
        *.datarate = default(this.bitrate);
        *.macTableModule = default(this.macTableModule);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        cutthroughSink: EthernetCutthroughSink {
            @display("p=200,100");
        }
        cutthroughSource: EthernetCutthroughSource {
            @display("p=500,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> cutthroughSink.in;
        cutthroughSink.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> cutthroughSource.in;
        cutthroughSource.out --> { @display("m=n"); } --> upperLayerOut;
}
