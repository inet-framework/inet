//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.measurement;

import inet.queueing.contract.IPacketFlow;

//
// Implements a protocol layer that measures packet flow performance metrics.
// Uses separate components for starting measurements on outgoing packets and
// recording results on incoming packets.
//
// Operation:
// - Outgoing path: Marks packets with measurement start information
// - Incoming path: Records measurement data when packets return
// - Uses configurable measurement starter and recorder modules
// - Supports various flow measurement techniques
// - Maintains bidirectional measurement capabilities
//
// Particularly useful for:
// - Measuring end-to-end packet delay
// - Calculating throughput and packet loss statistics
// - Analyzing protocol performance characteristics
// - Implementing network monitoring functionality
// - Supporting performance testing and validation
//
// @see ~IMeasurementLayer, ~MultiMeasurementLayer, ~OmittedMeasurementLayer
//
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
