//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.measurement;

//
// Defines the interface for protocol layers that perform packet flow measurements.
// Measurement layers collect statistics and performance metrics about packet flows
// passing through them.
//
// Operation:
// - Processes packets in both directions (upper to lower and lower to upper)
// - Collects timing, throughput, and other performance metrics
// - May tag packets with measurement data
// - Provides standardized measurement capabilities across protocol stacks
// - Can be inserted at any point in a protocol stack
//
// Particularly useful for:
// - Collecting network performance statistics
// - Analyzing protocol behavior and efficiency
// - Implementing Quality of Service (QoS) monitoring
// - Supporting network diagnostics and troubleshooting
// - Evaluating protocol stack performance
//
// @see ~MeasurementLayer, ~MultiMeasurementLayer, ~OmittedMeasurementLayer
//
moduleinterface IMeasurementLayer
{
    parameters:
        @omittedTypename(OmittedMeasurementLayer);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
