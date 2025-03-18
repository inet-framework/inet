//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.contract;

//
// Module interface for Ethernet MAC (Media Access Control) layer implementations.
// Defines the standard interface for various MAC layer variants in the modular
// Ethernet architecture. Provides connection points to upper layers (e.g., LLC)
// and lower layers (e.g., PHY) through standardized gates. Implementations
// include basic MAC, streaming MAC, preempting MAC, and fragmenting MAC, each
// offering different capabilities for frame transmission and processing.
//
moduleinterface IEthernetMacLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
