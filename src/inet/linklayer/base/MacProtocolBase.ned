//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.base;

import inet.common.LayeredProtocolBase;

//
// Provides a common base module for implementing Medium Access Control (MAC) protocols
// in the INET framework. This module handles the basic infrastructure needed by all
// MAC protocols, allowing derived modules to focus on protocol-specific behavior.
//
// Key features provided by this base module:
// - Standard gate structure for connecting to upper and lower layers
// - Integration with the network interface infrastructure
// - Packet queue management for outgoing frames
// - Lifecycle management (startup, shutdown, crash)
// - Common statistics collection
//
// MAC protocol implementations should extend this module and override the
// configureNetworkInterface() method to register protocol-specific capabilities
// with the network interface.
//
// @see ~IMacProtocol, ~LayeredProtocolBase
//
simple MacProtocolBase extends LayeredProtocolBase
{
    parameters:
        string interfaceTableModule;
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(INetworkHeader/down);
        output upperLayerOut @labels(INetworkHeader/up);
        input lowerLayerIn @labels(ILinkLayerFrame/up);
        output lowerLayerOut @labels(ILinkLayerFrame/down);
}
