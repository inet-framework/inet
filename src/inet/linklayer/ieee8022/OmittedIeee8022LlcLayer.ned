//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// This module is a placeholder that does not perform any function and is removed
// from the module hierarchy (i.e. replaced by a connection) during initialization.
// It can be used instead of making a submodule of the given module interface type
// conditional.
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
