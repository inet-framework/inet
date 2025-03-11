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
// Displays information about network interfaces in network nodes.
//
// The visualization includes:
// - Labels showing interface names, addresses, and status information
// - Customizable display format with various interface properties
// - Filtering options to visualize only specific interfaces or nodes
// - Automatic updates when interface properties change
//
// Particularly useful for understanding network topology at the interface level,
// debugging address assignment issues, verifying interface configurations, and
// creating informative visualizations of network connectivity in 3D view.
//
// @see ~InterfaceTableCanvasVisualizer, ~InterfaceTableVisualizer, ~InterfaceTableVisualizerBase, ~IInterfaceTableVisualizer
//
simple InterfaceTableOsgVisualizer extends InterfaceTableVisualizerBase like IInterfaceTableVisualizer
{
    parameters:
        @class(InterfaceTableOsgVisualizer);
}
