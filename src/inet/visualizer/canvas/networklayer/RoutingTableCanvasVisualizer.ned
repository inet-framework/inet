//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.visualizer.canvas.networklayer;

import inet.visualizer.base.RoutingTableVisualizerBase;
import inet.visualizer.contract.IRoutingTableVisualizer;

//
// Visualizes routes from routing tables on a 2D canvas. Displays arrows pointing
// from source nodes to next hop nodes with labels showing routing information.
//
// The visualization includes:
// - Arrows representing routes between nodes (source to next hop)
// - Labels showing route details in format "destination/mask -> gateway (interface)"
// - Dynamic updates when routing tables change (e.g., due to DHCP or routing protocols)
// - Filtering options to focus on specific nodes or destinations
//
// Particularly useful for understanding routing paths in complex networks,
// troubleshooting connectivity issues, and observing dynamic routing protocol behavior.
//
// @see ~RoutingTableOsgVisualizer, ~RoutingTableVisualizer, ~RoutingTableVisualizerBase, ~IRoutingTableVisualizer
//
simple RoutingTableCanvasVisualizer extends RoutingTableVisualizerBase like IRoutingTableVisualizer
{
    parameters:
        double zIndex = default(0); // Determines the drawing order of figures relative to other visualizers
        @class(RoutingTableCanvasVisualizer);
}
