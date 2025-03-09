//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.processing;

//
// Provides a placeholder implementation of the processing delay layer interface
// that can be used when processing delay modeling is not needed. Automatically
// removes itself from the module hierarchy during initialization and directly
// connects upper and lower layers.
//
// Operation:
// - Connects upper layer input directly to lower layer output
// - Connects lower layer input directly to upper layer output
// - Removes itself from the module hierarchy during initialization
// - Provides transparent packet flow between connected layers
//
// @see ~IProcessingDelayLayer, ~ProcessingDelayLayer
//
module OmittedProcessingDelayLayer like IProcessingDelayLayer
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/layer,black,50");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    connections:
        upperLayerIn --> lowerLayerOut;
        lowerLayerIn --> upperLayerOut;
}
