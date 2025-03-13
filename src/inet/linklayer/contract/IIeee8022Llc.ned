//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Defines the interface for IEEE 802.2 Logical Link Control (LLC) protocol
// implementations. The LLC sublayer provides the interface between the Media
// Access Control (MAC) sublayer and the network layer, enabling protocol
// multiplexing across different link layer technologies.
//
// Modules implementing this interface handle:
// - Protocol multiplexing using Service Access Point (SAP) identifiers
// - LLC header processing for both incoming and outgoing packets
// - Connection-less data link services (LLC Type 1)
// - Optional connection-oriented services (LLC Type 2, in some implementations)
//
// The interface specifies the minimum requirements for any LLC implementation,
// including the necessary gates for connecting to upper and lower layers.
// Upper layers typically include network protocols like IP, while lower layers
// include MAC protocols like Ethernet.
//
// @see ~Ieee8022Llc, ~Ieee8022LlcLayer
//
moduleinterface IIeee8022Llc
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
