//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.measurement;

import inet.queueing.contract.IPacketFlow;

module MeasurementLayer like IMeasurementLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        measurementStarter: <default("FlowMeasurementStarter")> like IPacketFlow {
            parameters:
                @display("p=150,100");
        }
        measurementRecorder: <default("FlowMeasurementRecorder")> like IPacketFlow {
            parameters:
                @display("p=350,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> measurementStarter.in;
        measurementStarter.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> measurementRecorder.in;
        measurementRecorder.out --> { @display("m=n"); } --> upperLayerOut;
}

