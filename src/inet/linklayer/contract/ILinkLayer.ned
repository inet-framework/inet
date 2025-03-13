//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Defines the interface for the upper part of link layer modules in the INET framework.
// This interface standardizes how link layer modules connect to and communicate with
// the network layer above them.
//
// The upper interface provides:
// - Input gate for receiving packets from the network layer
// - Output gate for sending packets to the network layer
// - Standardized labeling for proper packet type identification
//
// This interface is typically used by modules that need to interact with the
// network layer but may have different implementations of the lower part of
// the link layer.
//
// @see ~ILinkLayerLower, ~ILinkLayer
//
moduleinterface ILinkLayerUpper
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(INetworkHeader/down);
        output upperLayerOut @labels(INetworkHeader/up);
}

//
// Defines the interface for the lower part of link layer modules in the INET framework.
// This interface standardizes how link layer modules connect to and communicate with
// the physical layer or other lower layer modules below them.
//
// The lower interface provides:
// - Input gate for receiving frames from the physical layer
// - Output gate for sending frames to the physical layer
// - Standardized labeling for proper frame type identification
//
// This interface is typically used by modules that need to interact with the
// physical layer but may have different implementations of the upper part of
// the link layer.
//
// @see ~ILinkLayerUpper, ~ILinkLayer
//
moduleinterface ILinkLayerLower
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input lowerLayerIn @labels(ILinkFrame/up);
        output lowerLayerOut @labels(ILinkFrame/down);
}

//
// Defines a comprehensive interface for link layer modules in the INET framework
// by combining both upper and lower interface requirements. This interface
// provides a complete specification for modules that implement full link layer
// functionality, connecting the network layer above with the physical layer below.
//
// The combined interface includes:
// - Network layer connection points (from ILinkLayerUpper)
// - Physical layer connection points (from ILinkLayerLower)
// - All required gate labels for proper packet/frame identification
//
// This interface is implemented by various link layer protocols such as
// Ethernet, PPP, IEEE 802.11, and other MAC protocols that need to interact
// with both upper and lower layers.
//
// @see ~ILinkLayerUpper, ~ILinkLayerLower, ~IMacProtocol
//
moduleinterface ILinkLayer extends ILinkLayerUpper, ILinkLayerLower
{
    parameters:
        @display("i=block/rxtx");
}
