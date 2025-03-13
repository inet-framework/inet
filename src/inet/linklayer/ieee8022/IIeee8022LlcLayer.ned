//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee8022;

//
// Defines the interface for IEEE 802.2 Logical Link Control (LLC) layer
// implementations. This interface specifies the minimum requirements for
// any module that implements the LLC layer functionality as a compound module
// with potentially multiple internal components.
//
// The LLC layer serves as an interface between the network layer protocols
// and the MAC sublayer, providing protocol multiplexing capabilities through
// Service Access Point (SAP) identifiers. This allows multiple network layer
// protocols to share the same physical medium.
//
// Modules implementing this interface typically handle:
// - LLC header processing (insertion and checking)
// - SNAP header processing for extended protocol identification
// - Protocol multiplexing and demultiplexing
// - Optional socket support for direct LLC communication
//
// The interface specifies the standard gates for connecting to upper layers
// (typically network protocols) and lower layers (typically MAC protocols).
//
// @see ~Ieee8022LlcLayer, ~IIeee8022Llc
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
