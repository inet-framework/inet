//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.contract;

//
// Defines the interface for layer 2 node configurator modules that handle
// the configuration of link layer protocols and parameters within individual
// network nodes.
//
// Layer 2 node configurators are responsible for:
// - Applying configuration data from network-wide configurators to local node interfaces
// - Handling node lifecycle events (startup, shutdown, crash)
// - Configuring spanning tree protocol parameters
// - Setting up other link layer protocol parameters
//
// This interface allows for different implementations of node configurators
// while maintaining a consistent API for network nodes to interact with.
// Implementations typically connect to both the node's interface table and
// a global network configurator.
//
// @see ~L2NodeConfigurator, ~L2NetworkConfigurator
//
moduleinterface IL2NodeConfigurator
{
    parameters:
        @display("i=block/cogwheel");
}
