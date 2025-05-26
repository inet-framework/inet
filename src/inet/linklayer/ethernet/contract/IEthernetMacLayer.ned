//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.contract;

//
// Module interface for Ethernet MAC (Media Access Control) layer implementations.
// Defines an interface for various MAC layer variants in the modular Ethernet
// architecture. Provides connection points to upper layers (e.g., LLC)
// and lower layers (e.g., PHY) through standardized gates.
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
