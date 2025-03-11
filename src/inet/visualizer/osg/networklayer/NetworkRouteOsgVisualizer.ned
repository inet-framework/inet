//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.osg.networklayer;

import inet.visualizer.base.PathVisualizerBase;
import inet.visualizer.contract.INetworkRouteVisualizer;

//
// Visualizes network layer packet routes in a 3D scene using OpenSceneGraph (OSG).
// Displays the paths that packets take through the network at the IP layer.
//
// The visualization includes:
// - 3D polylines showing active network routes between nodes
// - Temporal fading as routes become inactive over time
// - Automatic updates when new packets travel along routes
// - Dynamic positioning that follows mobile node movement
// - Customizable appearance including colors, line styles, and fading parameters
//
// A route becomes active when a packet travels along a sequence of network nodes.
// The polyline representing this route gradually fades unless reinforced by
// subsequent packets following the same path.
//
// Particularly useful for understanding routing patterns, analyzing network layer
// connectivity, debugging routing protocols, and creating dynamic visualizations
// of network traffic flows in 3D view.
//
// @see ~NetworkRouteCanvasVisualizer, ~NetworkRouteVisualizer, ~PathVisualizerBase, ~INetworkRouteVisualizer
//
simple NetworkRouteOsgVisualizer extends PathVisualizerBase like INetworkRouteVisualizer
{
    parameters:
        @class(NetworkRouteOsgVisualizer);
}
