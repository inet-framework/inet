//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.common;

import inet.linklayer.configurator.contract.IL2NodeConfigurator;

//
// Provides node-level configuration for layer 2 protocols by acting as a bridge
// between individual network nodes and the global ~L2NetworkConfigurator module.
// Each network node that requires layer 2 configuration should contain an instance
// of this module.
//
// While the global ~L2NetworkConfigurator computes and stores configuration data
// (such as port priorities, link costs, and other spanning tree parameters),
// it doesn't directly apply these settings to the nodes. This module performs
// the actual configuration of the node's interfaces and tables at two key times:
// - During network initialization
// - After node restart events (following shutdowns or crashes)
//
// This architecture separates the configuration computation (centralized in
// ~L2NetworkConfigurator) from the configuration application (distributed in
// L2NodeConfigurator instances). This separation is crucial for handling node
// lifecycle events properly, as the global configurator has no awareness of
// individual node restarts.
//
// The module connects to both the interface table of the node and the global
// configurator to transfer the necessary configuration information.
//
// @see ~L2NetworkConfigurator, ~IL2NodeConfigurator
//
simple L2NodeConfigurator like IL2NodeConfigurator
{
    parameters:
        @display("i=block/cogwheel");
        string interfaceTableModule;
        string l2ConfiguratorModule = default("l2NetworkConfigurator"); // The absolute path to the ~L2NetworkConfigurator; use "" if there is no configurator
}
