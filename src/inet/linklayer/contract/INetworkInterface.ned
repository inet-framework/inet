//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Defines the fundamental interface for all network interfaces in the INET framework.
// Network interfaces serve as the connection points between network nodes and
// communication channels, providing standardized access to various link layer
// technologies.
//
// This base interface establishes the minimum requirements for any network interface:
// - Connection points to upper layers (typically network layer)
// - Common display properties for visualization
//
// More specialized network interfaces extend this interface with additional
// capabilities specific to their medium type (e.g., wired, wireless) and
// technology (e.g., Ethernet, WiFi, PPP).
//
// @see ~IWiredInterface, ~IWirelessInterface, ~ILoopbackInterface, ~ITunnelInterface
//
moduleinterface INetworkInterface
{
    parameters:
        @display("i=block/ifcard");
    gates:
        input upperLayerIn;
        output upperLayerOut;
}
