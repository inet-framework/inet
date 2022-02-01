//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.measurement;

import inet.queueing.contract.IPacketFlow;

module MultiMeasurementLayer like IMeasurementLayer
{
    parameters:
        @display("i=block/layer");
        int numMeasurementModules = default(1);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        measurementStarter[numMeasurementModules]: <default("FlowMeasurementStarter")> like IPacketFlow {
            parameters:
                @display("p=150,100,column,100");
        }
        measurementMaker[numMeasurementModules]: <default("FlowMeasurementRecorder")> like IPacketFlow {
            parameters:
                @display("p=350,100,column,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> measurementStarter[0].in;
        for i=0..numMeasurementModules-2 {
            measurementStarter[i].out --> { @display("m=n"); } --> measurementStarter[i+1].in;
        }
        measurementStarter[numMeasurementModules-1].out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> measurementMaker[numMeasurementModules-1].in;
        for i=0..numMeasurementModules-2 {
            measurementMaker[i+1].out --> { @display("m=n"); } --> measurementMaker[i].in;
        }
        measurementMaker[0].out --> { @display("m=n"); } --> upperLayerOut;
}

