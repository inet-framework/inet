//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.basic;

import inet.linklayer.contract.IEthernetLayer;

//
// Placeholder module that implements the Ethernet encapsulation interface but removes itself from the module 
// hierarchy during initialization. Provides a direct connection between upper and lower layers, effectively 
// bypassing Ethernet encapsulation/decapsulation when it's not needed in a simulation.
//
// @see ~EthernetEncapsulation, ~IEthernetLayer, ~EthernetMacPhy, ~EthernetCsmaMacPhy
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
