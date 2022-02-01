//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// This module implements the module given interface and can be used as an omitted
// optional module that removes itself from the module hierarchy during initialize.
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
