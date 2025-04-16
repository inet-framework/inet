//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.processing;

//
// Module interface for processing delay layers that model packet processing
// time in network devices. Implementations simulate the delay introduced by
// packet processing operations in both ingress and egress directions.
//
moduleinterface IProcessingDelayLayer
{
    parameters:
        @omittedTypename(OmittedProcessingDelayLayer);
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}

