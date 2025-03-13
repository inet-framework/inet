//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Defines the interface for Medium Access Control (MAC) protocol implementations
// in the INET framework. MAC protocols manage access to the shared communication
// medium, providing addressing, framing, and medium access coordination.
//
// The MAC protocol interface includes:
// - Connection points to the upper layer (typically LLC or network layer)
// - Connection points to the lower layer (typically physical layer)
// - Standardized gate labels for proper packet/frame identification
//
// MAC protocols are responsible for:
// - Encapsulating network packets into MAC frames
// - Addressing using MAC addresses
// - Error detection (and sometimes correction)
// - Medium access coordination to avoid or handle collisions
// - Optional features like QoS prioritization, security, and power management
//
// This interface is implemented by various MAC protocols such as Ethernet,
// IEEE 802.11, IEEE 802.15.4, and other specialized protocols like B-MAC,
// L-MAC, X-MAC for wireless sensor networks.
//
// @see ~ILinkLayer, ~MacProtocolBase
//
moduleinterface IMacProtocol
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(INetworkHeader/down);
        output upperLayerOut @labels(INetworkHeader/up);
        input lowerLayerIn @labels(ILinkLayerFrame/up);
        output lowerLayerOut @labels(ILinkLayerFrame/down);
}
