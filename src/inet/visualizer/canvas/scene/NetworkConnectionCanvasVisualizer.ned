//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.scene;

import inet.visualizer.base.NetworkConnectionVisualizerBase;
import inet.visualizer.contract.INetworkConnectionVisualizer;

//
// Visualizes network connections on a 2D canvas. Displays lines representing
// physical or logical connections between network nodes.
//
// The visualization includes:
// - Lines showing connections between network devices
// - Support for filtering which connections are displayed
// - Integration with the standard OMNeT++ connection visualization
//
// Note: This visualizer is disabled by default (nodeFilter="") because OMNeT++
// already displays network connections in its default visualization.
//
// Particularly useful when custom connection visualization is needed beyond
// what OMNeT++ provides by default.
//
// @see ~NetworkConnectionOsgVisualizer, ~NetworkConnectionVisualizer, ~NetworkConnectionVisualizerBase, ~INetworkConnectionVisualizer
//
simple NetworkConnectionCanvasVisualizer extends NetworkConnectionVisualizerBase like INetworkConnectionVisualizer
{
    parameters:
        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers
        nodeFilter = default(""); // Disabled because OMNeT++ displays network connections by default
        @class(NetworkConnectionCanvasVisualizer);
}
