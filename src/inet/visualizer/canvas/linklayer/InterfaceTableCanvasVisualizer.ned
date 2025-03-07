//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.linklayer;

import inet.visualizer.base.InterfaceTableVisualizerBase;
import inet.visualizer.contract.IInterfaceTableVisualizer;

//
// Visualizes network interface information on a 2D canvas. Displays labels with
// interface details such as names, IP addresses, and MAC addresses.
//
// The visualization includes:
// - Interface names and IP addresses displayed above nodes or on links
// - Configurable format to show different combinations of interface properties
// - Automatic updates when interfaces obtain new addresses (e.g., via DHCP)
// - Filtering options to show only specific interfaces or nodes
// - Placement options for wired interfaces (on connections or above nodes)
//
// Particularly useful for verifying network configuration, debugging address
// assignment, and getting a quick overview of network addressing.
//
// @see ~InterfaceTableOsgVisualizer, ~InterfaceTableVisualizer, ~InterfaceTableVisualizerBase, ~IInterfaceTableVisualizer
//
simple InterfaceTableCanvasVisualizer extends InterfaceTableVisualizerBase like IInterfaceTableVisualizer
{
    parameters:
        double zIndex = default(10); // Determines the drawing order of figures relative to other visualizers
        string anchor @enum("l","r","t") = default("l"); // Specifies the beginning/end/middle
        @class(InterfaceTableCanvasVisualizer);
}
