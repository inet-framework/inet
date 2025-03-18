//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for Ethernet protocol layer implementations. Provides
// functionality for handling Ethernet frames including MAC header processing,
// padding to minimum frame size, and frame check sequence (FCS) calculation and
// verification. Implementations typically handle the encapsulation of higher layer
// packets into Ethernet frames for transmission and the decapsulation of received
// Ethernet frames for delivery to higher layers. May also support socket-based
// access to Ethernet services for direct communication with applications.
//
moduleinterface IEthernetLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
