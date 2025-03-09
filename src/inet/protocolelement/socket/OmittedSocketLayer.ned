//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.socket;

//
// Implements a placeholder for an optional socket layer that removes itself
// from the module hierarchy during initialization, creating a direct connection
// between adjacent protocol layers.
//
// Operation:
// - Connects upper and lower layer interfaces directly
// - Automatically removes itself during initialization phase
// - Maintains proper packet flow between connected layers
// - Preserves the logical structure of the protocol stack
// - Allows for simplified protocol stack configurations
//
// @see ~ISocketLayer
//
module OmittedSocketLayer like ISocketLayer
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
