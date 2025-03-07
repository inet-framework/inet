//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.linklayer;

import inet.visualizer.base.InterfaceTableVisualizerBase;
import inet.visualizer.contract.IInterfaceTableVisualizer;

//
// Visualizes network interfaces in a 3D scene using OpenSceneGraph (OSG).
// Displays information about network interface cards (NICs) in network nodes.
//
// The visualization includes:
// - 3D labels showing interface names, addresses, and status information
// - Customizable display format with various interface properties
// - Filtering options to visualize only specific interfaces or nodes
// - Support for all interface types (Ethernet, Wi-Fi, PPP, etc.)
// - Automatic updates when interface properties change
//
// Particularly useful for understanding network topology at the interface level,
// debugging address assignment issues, verifying interface configurations, and
// creating informative visualizations of network connectivity in 3D space.
//
// @see ~InterfaceTableCanvasVisualizer, ~InterfaceTableVisualizer, ~InterfaceTableVisualizerBase, ~IInterfaceTableVisualizer
//
simple InterfaceTableOsgVisualizer extends InterfaceTableVisualizerBase like IInterfaceTableVisualizer
{
    parameters:
        @class(InterfaceTableOsgVisualizer);
}
