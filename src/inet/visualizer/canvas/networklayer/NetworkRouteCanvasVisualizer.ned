//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.networklayer;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.INetworkRouteVisualizer;

//
// Visualizes active network paths on a 2D canvas. Displays polyline arrows
// showing the complete path that packets take through the network at the network layer.
//
// The visualization includes:
// - Polyline arrows following the entire path from source to destination through all intermediate nodes
// - Packet names displayed on the arrows
// - Gradual fading of paths that are no longer active
// - Dynamic updates when routes change due to mobility or network topology changes
// - Filtering options to visualize only specific packet types or nodes
//
// Particularly useful for understanding multi-hop communication paths, analyzing
// routing protocol behavior in dynamic networks, and troubleshooting connectivity issues.
//
// @see ~NetworkRouteOsgVisualizer, ~NetworkRouteVisualizer, ~PathVisualizerBase, ~INetworkRouteVisualizer
//
simple NetworkRouteCanvasVisualizer extends PathVisualizerBase like INetworkRouteVisualizer
{
    parameters:
        double zIndex = default(3); // Determines the drawing order of figures relative to other visualizers
        @class(NetworkRouteCanvasVisualizer);
}
