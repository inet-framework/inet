//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

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

