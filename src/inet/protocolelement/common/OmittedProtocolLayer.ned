//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.protocolelement.common;

import inet.protocolelement.contract.IProtocolLayer;

//
// Implements a placeholder for an optional protocol layer that removes itself
// from the module hierarchy during initialization, creating a direct connection
// between adjacent layers.
//
// Operation:
// - Connects upper and lower layer interfaces directly
// - Automatically removes itself during initialization phase
// - Maintains proper signal propagation between connected layers
// - Preserves the logical structure of the protocol stack
//
// @see ~IProtocolLayer
//
module OmittedProtocolLayer like IProtocolLayer
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
