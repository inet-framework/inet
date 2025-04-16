//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.measurement;

//
// Module interface for measurement layers that collect performance metrics
// on packet flows. Implementations monitor and record various statistics
// about packets passing through the network stack.
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

