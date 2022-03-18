//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.basic;

import inet.linklayer.ethernet.contract.IEthernetLayer;

//
// This module implements the module given interface and can be used as an omitted
// optional module that removes itself from the module hierarchy during initialize.
//
module OmittedEthernetEncapsulation like IEthernetLayer
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

