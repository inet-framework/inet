//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.transportlayer;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.ITransportRouteVisualizer;

//
// Visualizes transport layer packet paths on a 2D canvas. Displays polyline arrows
// showing the complete path that packets take through the network at the transport layer.
//
// The visualization includes:
// - Polyline arrows following the path from source to destination
// - Separate arrows for each direction when bidirectional communication occurs
// - Gradual fading of paths that are no longer active
// - Dynamic updates when routes change due to mobility or network topology changes
// - Filtering options to visualize only specific packet types or nodes
//
// Particularly useful for understanding end-to-end communication patterns,
// analyzing transport protocol behavior, and troubleshooting application-level
// connectivity issues.
//
// @see ~TransportRouteOsgVisualizer, ~TransportRouteVisualizer, ~PathVisualizerBase, ~ITransportRouteVisualizer
//
simple TransportRouteCanvasVisualizer extends PathVisualizerBase like ITransportRouteVisualizer
{
    parameters:
        double zIndex = default(4); // Determines the drawing order of figures relative to other visualizers
        @class(TransportRouteCanvasVisualizer);
}
