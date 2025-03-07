//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.scene;

import inet.visualizer.base.NetworkNodeVisualizerBase;
import inet.visualizer.contract.INetworkNodeVisualizer;

//
// Visualizes network nodes on a 2D canvas. Creates and maintains visual
// representations of network devices that serve as the foundation for
// other visualizations.
//
// The visualization includes:
// - Graphical figures representing each network node in the simulation
// - Support for node filtering to display only specific nodes
// - A framework for other visualizers to attach annotations to nodes
// - Automatic updates when nodes are created, moved, or removed
//
// Particularly useful as the base visualization layer upon which other
// visualizers build, providing the core representation of network topology.
//
// @see ~NetworkNodeOsgVisualizer, ~NetworkNodeVisualizer, ~NetworkNodeVisualizerBase, ~INetworkNodeVisualizer
//
simple NetworkNodeCanvasVisualizer extends NetworkNodeVisualizerBase like INetworkNodeVisualizer
{
    parameters:
        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers
        @class(NetworkNodeCanvasVisualizer);
}
