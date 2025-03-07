//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.base;

//
// Serves as a base module for network node visualizer modules. Visualizes network
// nodes (routers, hosts, etc.) as graphical objects in the simulation. Provides
// common functionality for filtering which nodes are displayed and managing
// the placement of annotations around nodes.
//
// Network node visualizers are fundamental visualizers that create the visual
// representations of network devices. Other visualizers often attach their
// visualizations to these node representations (e.g., adding interface icons,
// mobility trails, or connection lines between nodes).
//
// The actual rendering of network nodes is implemented in derived modules for
// specific user interfaces (canvas or OSG).
//
// @see ~NetworkNodeCanvasVisualizer, ~NetworkNodeOsgVisualizer, ~INetworkNodeVisualizer, ~VisualizerBase
//
simple NetworkNodeVisualizerBase extends VisualizerBase
{
    parameters:
        string nodeFilter @mutable = default("*"); // Determines which network nodes are displayed. All nodes are displayed by default. The filter can use wildcards and expressions to select specific nodes.

        double annotationSpacing = default(4); // Spacing between annotations attached to network nodes, in pixels
        double placementPenalty = default(10); // Penalty value used in the annotation placement algorithm to avoid overlapping annotations

        @class(NetworkNodeVisualizerBase);
}
