//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee8022;

//
// Module interface for IEEE 802.2 Logical Link Control (LLC) layer implementations.
// Defines the interface for modules that process packets from upper layers by
// adding LLC headers before forwarding to lower layers, and process packets from
// lower layers by checking and removing LLC headers before forwarding to upper
// layers. Implementations may support both standard LLC headers and SubNetwork
// Access Protocol (SNAP) headers.
//
moduleinterface IIeee8022LlcLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
