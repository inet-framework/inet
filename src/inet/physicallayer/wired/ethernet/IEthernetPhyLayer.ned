//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wired.ethernet;

//
// Interface for Ethernet PHY layer implementations. Defines the common gates
// and basic structure that all Ethernet physical layer modules must implement.
// Various implementations can provide different capabilities such as streaming,
// preemption, or cut-through transmission.
//
moduleinterface IEthernetPhyLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
