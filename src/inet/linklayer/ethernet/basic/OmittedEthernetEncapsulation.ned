//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.basic;

import inet.common.Module;
import inet.linklayer.contract.IEthernetLayer;

//
// Implements the given module interface and can be used as an optional
// module that removes itself from the module hierarchy during initialization.
//
module OmittedEthernetEncapsulation extends Module like IEthernetLayer
{
    parameters:
        @class(::inet::OmittedModule);
        @display("i=block/square,black,50");
    gates:
        input upperLayerIn @labels(EtherFrame);
        output upperLayerOut @labels(EtherFrame);
        input lowerLayerIn @labels(EtherFrame);
        output lowerLayerOut @labels(EtherFrame);
    connections:
        upperLayerIn --> lowerLayerOut;
        lowerLayerIn --> upperLayerOut;
}

