//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// Placeholder module that implements the IEEE 802.2 LLC layer interface but removes itself from the 
// module hierarchy during initialization. Provides a direct connection between upper and lower layers 
// when LLC layer functionality is not needed. Useful in network configurations where the LLC layer is 
// optional or when protocol stacks need to be configurable with or without LLC processing.
//
// @see ~IIeee8022LlcLayer, ~Ieee8022LlcLayer, ~OmittedModule
//
module OmittedIeee8022LlcLayer like IIeee8022LlcLayer
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
