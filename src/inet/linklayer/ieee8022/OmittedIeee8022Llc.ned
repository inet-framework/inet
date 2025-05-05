//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

import inet.common.Module;
import inet.linklayer.contract.IIeee8022Llc;

//
// Implements the given module interface and can be used as an optional
// module that removes itself from the module hierarchy during initialization.
//
module OmittedIeee8022Llc extends Module like IIeee8022Llc
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

